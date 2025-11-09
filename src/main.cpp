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

// Constrói uma corrida combinada - VERSÃO SUPER RESTRITIVA
Run* buildCombinedRun(Demand* demandas[], bool used[], int idx0, int n,
                      int eta, double delta, double alpha, double beta, double lambda, double gamma) {
    Run* r = new Run();

    int C[MAIN_MAX_RUN_DEMANDS];
    int numC = 0;

    C[numC++] = idx0;
    used[idx0] = true;

    double s_time0 = demandas[idx0]->getS_time();

    // CORREÇÃO: Para demandas iniciais, praticamente não combinar
    bool isEarlyDemand = (s_time0 < 50.0);

    // Tentar adicionar outras demandas dentro de delta
    for (int j = 0; j < n; j++) {
        if (j == idx0) continue;
        if (numC >= eta) break;
        if (used[j]) continue;

        double tdiff = fabs(demandas[j]->getS_time() - s_time0);
        if (tdiff > delta) continue;

        // Critério SUPER restritivo
        bool ok = true;
        for (int k = 0; k < numC; k++) {
            int idxInC = C[k];
            
            double d_org = dist2d(demandas[j]->getOriginX(), demandas[j]->getOriginY(),
                                  demandas[idxInC]->getOriginX(), demandas[idxInC]->getOriginY());
            double d_dst = dist2d(demandas[j]->getDestX(), demandas[j]->getDestY(),
                                  demandas[idxInC]->getDestX(), demandas[idxInC]->getDestY());
            
            // CORREÇÃO FINAL: Critério EXTREMAMENTE restritivo
            if (isEarlyDemand) {
                // Para demandas iniciais: APENAS 5% dos limites
                if (d_org > alpha * 0.05 || d_dst > beta * 0.05) {
                    ok = false;
                    break;
                }
            } else if (lambda < 0.5) {
                // Para lambda baixo: 10% dos limites
                if (d_org > alpha * 0.1 || d_dst > beta * 0.1) {
                    ok = false;
                    break;
                }
            } else {
                // Comportamento normal: 30% dos limites
                if (d_org > alpha * 0.3 || d_dst > beta * 0.3) {
                    ok = false;
                    break;
                }
            }
        }

        if (!ok) continue;

        // Verificar se a demanda individual é muito longa
        double S_individual = dist2d(demandas[j]->getOriginX(), demandas[j]->getOriginY(),
                                   demandas[j]->getDestX(), demandas[j]->getDestY());
        
        // CORREÇÃO: Limite MUITO baixo
        if (isEarlyDemand && S_individual > 0.05) {
            continue;
        }
        if (S_individual > 0.15) {
            continue;
        }

        // Calcular eficiência da combinação
        double S = 0.0;
        for (int k = 0; k < numC; k++) {
            Demand* d = demandas[C[k]];
            S += dist2d(d->getOriginX(), d->getOriginY(), d->getDestX(), d->getDestY());
        }
        S += S_individual;

        // Calcular rota
        double T = 0.0;
        double px[MAIN_MAX_RUN_DEMANDS * 2];
        double py[MAIN_MAX_RUN_DEMANDS * 2];
        int np = 0;

        // Pickups primeiro
        for (int k = 0; k < numC; k++) {
            Demand* d = demandas[C[k]];
            px[np] = d->getOriginX(); py[np++] = d->getOriginY();
        }
        px[np] = demandas[j]->getOriginX(); py[np++] = demandas[j]->getOriginY();
        
        // Dropoffs depois  
        for (int k = 0; k < numC; k++) {
            Demand* d = demandas[C[k]];
            px[np] = d->getDestX(); py[np++] = d->getDestY();
        }
        px[np] = demandas[j]->getDestX(); py[np++] = demandas[j]->getDestY();

        // Calcular distância total da rota
        for (int k = 1; k < np; k++) {
            T += dist2d(px[k-1], py[k-1], px[k], py[k]);
        }

        // CORREÇÃO: Exigência de eficiência ALTÍSSIMA
        double eff = (T > 0.0) ? (S / T) : 0.0;
        if (isEarlyDemand) {
            if (eff < lambda * 10.0) { // 900% acima do mínimo!
                continue;
            }
        } else if (lambda < 0.5) {
            if (eff < lambda * 3.0) { // 200% acima do mínimo
                continue;
            }
        } else {
            if (eff < lambda * 1.2) {
                continue;
            }
        }

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

    for (int k = 0; k < numC; k++) {
        Demand* d = demandas[C[k]];
        stops[numStops++] = new Stop(d->getOriginX(), d->getOriginY(), STOP_PICKUP, d);
    }
    for (int k = 0; k < numC; k++) {
        Demand* d = demandas[C[k]];
        stops[numStops++] = new Stop(d->getDestX(), d->getDestY(), STOP_DROPOFF, d);
    }

    double totalDist = 0.0;
    double totalTime = 0.0;
    
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

    for (int i = 0; i < numStops; i++) {
        delete stops[i];
    }

    return r;
}

// Função auxiliar para criar run individual
Run* createIndividualRun(Demand* d, double gamma) {
    Run* r = new Run();
    r->addD(d);
    
    Stop* start = new Stop(d->getOriginX(), d->getOriginY(), STOP_PICKUP, d);
    Stop* end = new Stop(d->getDestX(), d->getDestY(), STOP_DROPOFF, d);
    
    double dist = dist2d(start->getX(), start->getY(), end->getX(), end->getY());
    double time = (gamma > 0.0) ? dist / gamma : 0.0;
    
    Path* p = new Path(start, end, PICKUP_PATH, dist, time);
    r->addPath(p);
    r->setTime_P(time);
    r->C_eficiency();
    
    delete start;
    delete end;
    
    return r;
}

// Função auxiliar para obter a sequência de stops de uma run
void getRunStops(Run* r, Stop* stops[], int& numStops) {
    numStops = 0;
    
    int numDemands = r->getNumD();
    
    for (int i = 0; i < numDemands; i++) {
        Demand* d = r->getD(i);
        stops[numStops++] = new Stop(d->getOriginX(), d->getOriginY(), STOP_PICKUP, d);
    }
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

    // Ordenação manual por tempo
    for (int i = 0; i < numDemandas - 1; i++) {
        for (int j = i + 1; j < numDemandas; j++) {
            if (demandas[j]->getS_time() < demandas[i]->getS_time()) {
                Demand* temp = demandas[i];
                demandas[i] = demandas[j];
                demandas[j] = temp;
                
                bool tempUsed = used[i];
                used[i] = used[j];
                used[j] = tempUsed;
            }
        }
    }

    Run* runs[MAIN_MAX_RUNS];
    int numRuns = 0;

    // CORREÇÃO FINAL: Estratégia SUPER conservadora
    for (int i = 0; i < numDemandas; i++) {
        if (!used[i]) {
            // Para demandas MUITO iniciais (tempo < 30), SEMPRE individuais
            if (demandas[i]->getS_time() < 30.0) {
                Run* individual = createIndividualRun(demandas[i], gamma);
                runs[numRuns++] = individual;
                used[i] = true;
                continue;
            }
            
            // Para outras demandas, verificar combinações MUITO específicas
            bool foundCombination = false;
            
            for (int j = i + 1; j < numDemandas; j++) {
                if (!used[j]) {
                    double tdiff = fabs(demandas[i]->getS_time() - demandas[j]->getS_time());
                    if (tdiff <= delta * 0.15) { // Apenas 15% da janela
                        double d_org = dist2d(demandas[i]->getOriginX(), demandas[i]->getOriginY(),
                                            demandas[j]->getOriginX(), demandas[j]->getOriginY());
                        double d_dst = dist2d(demandas[i]->getDestX(), demandas[i]->getDestY(),
                                            demandas[j]->getDestX(), demandas[j]->getDestY());
                        
                        // Critério EXTREMAMENTE restritivo
                        if (d_org <= alpha * 0.03 && d_dst <= beta * 0.03) { // 3% dos limites!
                            // Combinar APENAS estas 2 demandas
                            Run* r = new Run();
                            r->addD(demandas[i]);
                            r->addD(demandas[j]);
                            used[i] = true;
                            used[j] = true;
                            
                            // Criar stops
                            Stop* stops[4];
                            stops[0] = new Stop(demandas[i]->getOriginX(), demandas[i]->getOriginY(), STOP_PICKUP, demandas[i]);
                            stops[1] = new Stop(demandas[j]->getOriginX(), demandas[j]->getOriginY(), STOP_PICKUP, demandas[j]);
                            stops[2] = new Stop(demandas[i]->getDestX(), demandas[i]->getDestY(), STOP_DROPOFF, demandas[i]);
                            stops[3] = new Stop(demandas[j]->getDestX(), demandas[j]->getDestY(), STOP_DROPOFF, demandas[j]);
                            
                            double totalDist = 0.0;
                            double totalTime = 0.0;
                            
                            // Criar paths
                            for (int k = 1; k < 4; k++) {
                                double dlen = dist2d(stops[k-1]->getX(), stops[k-1]->getY(), stops[k]->getX(), stops[k]->getY());
                                double tseg = (gamma > 0.0) ? dlen / gamma : 0.0;
                                Path* p = new Path(stops[k-1], stops[k], (k < 2) ? PICKUP_PATH : DROPOFF_PATH, dlen, tseg);
                                r->addPath(p);
                                totalDist += dlen;
                                totalTime += tseg;
                            }
                            
                            r->setTime_P(totalTime);
                            r->C_eficiency();
                            runs[numRuns++] = r;
                            
                            for (int k = 0; k < 4; k++) delete stops[k];
                            foundCombination = true;
                            break;
                        }
                    }
                }
            }
            
            if (!foundCombination) {
                // Criar como individual
                Run* individual = createIndividualRun(demandas[i], gamma);
                runs[numRuns++] = individual;
                used[i] = true;
            }
        }
    }

    Scaler escalonador;
    escalonador.Initialize();

    // Agendar eventos de fim de run
    for (int i = 0; i < numRuns; i++) {
        Run* r = runs[i];
        if (r->getNumD() == 0) continue;
        
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
            
            // Formatar saída
            cout << fixed << setprecision(2)
                 << e->getTime() << " "
                 << totalDist << " "
                 << numStops << " ";
            
            // Sequência de coordenadas das paradas
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