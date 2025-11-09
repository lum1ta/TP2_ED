#include <iostream>
#include <iomanip>
#include <cmath>
#include "Scaler.h"
#include "Demand.h"
#include "Run.h"
#include "Path.h"
#include "Stop.h"
#include "Event.h"

using namespace std;

#define MAIN_MAX_DEMANDS 1000
#define MAIN_MAX_RUNS 500
#define MAIN_MAX_PATHS 20
#define MAIN_MAX_RUN_DEMANDS 10

// Função auxiliar: distância euclidiana
double dist2d(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx*dx + dy*dy);
}

// Constrói uma corrida combinada
Run* buildCombinedRun(Demand* demandas[], bool used[], int idx0, int n,
                      int eta, double delta, double alpha, double beta, double lambda, double gamma) {
    Run* r = new Run();

    int C[MAIN_MAX_RUN_DEMANDS];
    int numC = 0;

    C[numC++] = idx0;
    used[idx0] = true;

    double s_time0 = demandas[idx0]->getS_time();

    // Tentar adicionar outras demandas dentro de delta
    for (int j = idx0 + 1; j < n; j++) {
        if (numC >= eta) break;
        if (used[j]) continue;

        double tdiff = demandas[j]->getS_time() - s_time0;
        if (tdiff > delta) continue;

        // Verificar distâncias conjuntas entre origens e destinos
        bool ok = true;
        for (int k = 0; k < numC; k++) {
            int idxInC = C[k];
            double d_org = dist2d(demandas[j]->getOriginX(), demandas[j]->getOriginY(),
                                  demandas[idxInC]->getOriginX(), demandas[idxInC]->getOriginY());
            double d_dst = dist2d(demandas[j]->getDestX(), demandas[j]->getDestY(),
                                  demandas[idxInC]->getDestX(), demandas[idxInC]->getDestY());
            if (d_org > alpha || d_dst > beta) {
                ok = false;
                break;
            }
        }

        if (!ok) continue;

        // Calcular eficiência provisória (S/T)
        double S = 0.0;
        for (int k = 0; k < numC; k++) {
            Demand* d = demandas[C[k]];
            S += dist2d(d->getOriginX(), d->getOriginY(), d->getDestX(), d->getDestY());
        }
        S += dist2d(demandas[j]->getOriginX(), demandas[j]->getOriginY(),
                    demandas[j]->getDestX(), demandas[j]->getDestY());

        // rota (pickups, depois dropoffs)
        double T = 0.0;
        double px[MAIN_MAX_RUN_DEMANDS * 2];
        double py[MAIN_MAX_RUN_DEMANDS * 2];
        int np = 0;

        // Ordem correta - todos pickups primeiro, depois todos dropoffs
        for (int k = 0; k < numC; k++) {
            Demand* d = demandas[C[k]];
            px[np] = d->getOriginX(); py[np++] = d->getOriginY();
        }
        px[np] = demandas[j]->getOriginX(); py[np++] = demandas[j]->getOriginY();
        
        for (int k = 0; k < numC; k++) {
            Demand* d = demandas[C[k]];
            px[np] = d->getDestX(); py[np++] = d->getDestY();
        }
        px[np] = demandas[j]->getDestX(); py[np++] = demandas[j]->getDestY();

        // Calcular distância total da rota
        for (int k = 1; k < np; k++) {
            T += dist2d(px[k-1], py[k-1], px[k], py[k]);
        }

        double eff = (T > 0.0) ? (S / T) : 0.0;
        if (eff < lambda) continue;

        // Adicionar demanda
        C[numC++] = j;
        used[j] = true;
    }

    // Construir efetivamente a corrida
    for (int k = 0; k < numC; k++) {
        r->addD(demandas[C[k]]);
    }

    // Criar stops e paths
    Stop* stops[MAIN_MAX_PATHS];
    int numStops = 0;

    // pickups (na ordem das demandas combinadas)
    for (int k = 0; k < numC; k++) {
        Demand* d = demandas[C[k]];
        stops[numStops++] = new Stop(d->getOriginX(), d->getOriginY(), STOP_PICKUP, d);
    }
    // dropoffs (mesma ordem dos pickups)
    for (int k = 0; k < numC; k++) {
        Demand* d = demandas[C[k]];
        stops[numStops++] = new Stop(d->getDestX(), d->getDestY(), STOP_DROPOFF, d);
    }

    double totalDist = 0.0;
    double totalTime = 0.0;
    
    // Criar paths entre os stops
    for (int i = 1; i < numStops; i++) {
        Path_Type type = (i < numC) ? PICKUP_PATH : DROPOFF_PATH;
        
        double dlen = dist2d(stops[i-1]->getX(), stops[i-1]->getY(),
                             stops[i]->getX(), stops[i]->getY());
        double tseg = (gamma > 0.0) ? dlen / gamma : 0.0;
        
        Path* p = new Path(stops[i-1], stops[i], type, dlen, tseg);
        r->addPath(p);
        totalDist += dlen;
        totalTime += tseg;
    }

    r->setTime_P(totalTime);
    r->C_eficiency();

    // Liberar memória dos stops
    for (int i = 0; i < numStops; i++) {
        delete stops[i];
    }

    return r;
}

// Função auxiliar para obter a sequência de stops de uma run
void getRunStops(Run* r, Stop* stops[], int& numStops) {
    numStops = 0;
    
    // Recriar a sequência de stops: todos pickups primeiro, depois todos dropoffs
    int numDemands = r->getNumD();
    
    // Pickups
    for (int i = 0; i < numDemands; i++) {
        Demand* d = r->getD(i);
        stops[numStops++] = new Stop(d->getOriginX(), d->getOriginY(), STOP_PICKUP, d);
    }
    
    // Dropoffs (na mesma ordem)
    for (int i = 0; i < numDemands; i++) {
        Demand* d = r->getD(i);
        stops[numStops++] = new Stop(d->getDestX(), d->getDestY(), STOP_DROPOFF, d);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int eta;
    double gamma, delta, alpha, beta, lambda;
    int n;

    // Ler parâmetros
    if (!(cin >> eta >> gamma >> delta >> alpha >> beta >> lambda)) {
        cerr << "Erro na leitura dos parâmetros.\n";
        return 1;
    }

    if (!(cin >> n)) {
        cerr << "Erro na leitura do número de demandas.\n";
        return 1;
    }

    Demand* demandas[MAIN_MAX_DEMANDS];
    bool used[MAIN_MAX_DEMANDS] = {false};
    int numDemandas = 0;

    // Ler demandas
    for (int i = 0; i < n; i++) {
        int id;
        double s_time, originX, originY, destX, destY;
        
        if (!(cin >> id >> s_time >> originX >> originY >> destX >> destY)) {
            cerr << "Erro na leitura da demanda " << i << endl;
            return 1;
        }
        
        demandas[numDemandas] = new Demand(id, s_time, originX, originY, destX, destY);
        used[numDemandas] = false;
        numDemandas++;
    }

    Run* runs[MAIN_MAX_RUNS];
    int numRuns = 0;

    // Construir runs combinadas
    for (int i = 0; i < numDemandas; i++) {
        if (!used[i]) {
            Run* r = buildCombinedRun(demandas, used, i, numDemandas,
                                      eta, delta, alpha, beta, lambda, gamma);
            if (r->getNumD() > 0) {
                runs[numRuns++] = r;
            } else {
                delete r;
            }
        }
    }

    Scaler escalonador;
    escalonador.Initialize();

    // Agendar eventos de fim de run
    for (int i = 0; i < numRuns; i++) {
        Run* r = runs[i];
        if (r->getNumD() == 0) continue;
        
        // Usar o tempo da PRIMEIRA demanda (idx0) em vez do maior tempo
        Demand* firstDemand = r->getD(0);
        double startTime = firstDemand->getS_time();
        double endTime = startTime + r->getTime_P();
        
        Path* firstPath = (r->getNumPaths() > 0) ? r->getPath(0) : nullptr;
        Event* e = new Event(endTime, END_RUN, firstPath, firstDemand, r);
        escalonador.Insert(e);
    }

    // Processar eventos
    while (true) {
        Event* e = escalonador.RemoveNextEvent();
        if (e == nullptr) break;

        if (e->getType() == END_RUN) {
            Run* r = e->getRun();
            
            // Calcular distância total
            double totalDist = 0.0;
            for (int j = 0; j < r->getNumPaths(); j++) {
                Path* p = r->getPath(j);
                totalDist += p->getDistance();
            }
            
            // Obter a sequência de stops (paradas)
            Stop* stops[MAIN_MAX_PATHS];
            int numStops = 0;
            getRunStops(r, stops, numStops);
            
            // Formatar saída com PARADAS
            cout << fixed << setprecision(2)
                 << e->getTime() << " "
                 << totalDist << " "
                 << numStops << " ";
            
            // Sequência de coordenadas das PARADAS
            for (int j = 0; j < numStops; j++) {
                cout << fixed << setprecision(2)
                     << stops[j]->getX() << " "
                     << stops[j]->getY();
                if (j != numStops - 1) cout << " ";
            }
            cout << "\n";
            
            // Liberar stops temporários
            for (int j = 0; j < numStops; j++) {
                delete stops[j];
            }
        }

        delete e;
    }

    escalonador.Finish();

    // Liberar memória
    for (int i = 0; i < numRuns; i++) delete runs[i];
    for (int i = 0; i < numDemandas; i++) delete demandas[i];

    return 0;
}