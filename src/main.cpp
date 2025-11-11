#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
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

double dist2d(double x1, double y1, double x2, double y2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}


//Função getRunStops 
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

// **VOLTAR ao critério RIGOROSO original**
bool canCombineWithGroup(Demand* newDemand, Demand* group[], int groupSize, 
                         double delta, double alpha, double beta) {
    // Critério de Tempo
    for (int i = 0; i < groupSize; i++) {
        if (fabs(newDemand->getS_time() - group[i]->getS_time()) > delta) 
            return false;
    }
    // Critério de distância entre origens (alfa)
    for (int i = 0; i < groupSize; i++) {
        double orgDist = dist2d(newDemand->getOriginX(), newDemand->getOriginY(),
                                group[i]->getOriginX(), group[i]->getOriginY());
        if (orgDist > alpha) return false;
    }
    // Critério de distância entre (beta)
    for (int i = 0; i < groupSize; i++) {
        double destDist = dist2d(newDemand->getDestX(), newDemand->getDestY(),
                                 group[i]->getDestX(), group[i]->getDestY());
        if (destDist > beta) return false;
    }
    // Adicionar: Critério de capacidade do carro
    return true;
}

// [CORREÇÃO 2: Rota da Versão Básica (PDF )]
double calculateActualRouteDistance(Demand* demandas[], int indices[], int count) {
    if (count == 0) return 0.0;
    if (count == 1) {
        return dist2d(demandas[indices[0]]->getOriginX(), demandas[indices[0]]->getOriginY(),
                      demandas[indices[0]]->getDestX(), demandas[indices[0]]->getDestY());
    }
    
    double totalDist = 0.0;

    // 1. Trechos de COLETA (P0 -> P1 -> ... -> P(k-1))
    for (int i = 1; i < count; i++) {
        totalDist += dist2d(demandas[indices[i-1]]->getOriginX(), // P(i-1)
                            demandas[indices[i-1]]->getOriginY(),
                            demandas[indices[i]]->getOriginX(),   // P(i)
                            demandas[indices[i]]->getOriginY());
    }
    
    // 2. Trecho de DESLOCAMENTO (P(k-1) -> D0)
    // P(k-1) = última coleta (indice count-1)
    // D0     = primeira entrega (indice 0)
    totalDist += dist2d(demandas[indices[count-1]]->getOriginX(), // P(k-1)
                        demandas[indices[count-1]]->getOriginY(),
                        demandas[indices[0]]->getDestX(),       // D0
                        demandas[indices[0]]->getDestY());
    
    // 3. Trechos de ENTREGA (D0 -> D1 -> ... -> D(k-1))
    for (int i = 1; i < count; i++) {
        totalDist += dist2d(demandas[indices[i-1]]->getDestX(), // D(i-1)
                            demandas[indices[i-1]]->getDestY(),
                            demandas[indices[i]]->getDestX(),   // D(i)
                            demandas[indices[i]]->getDestY());
    }
    
    return totalDist;
}
// [CORREÇÃO 3: Tipos de Trechos (PDF )]
Run* createOptimizedRun(Demand* demandas[], int indices[], int count, double gamma) {
    Run* r = new Run();
    for (int i = 0; i < count; i++) r->addD(demandas[indices[i]]);
    
    if (count == 1) {
        double dist = dist2d(demandas[indices[0]]->getOriginX(), demandas[indices[0]]->getOriginY(),
                             demandas[indices[0]]->getDestX(), demandas[indices[0]]->getDestY());
        double time = dist / gamma;
        Stop* start = new Stop(demandas[indices[0]]->getOriginX(), demandas[indices[0]]->getOriginY(), STOP_PICKUP, demandas[indices[0]]);
        Stop* end = new Stop(demandas[indices[0]]->getDestX(), demandas[indices[0]]->getDestY(), STOP_DROPOFF, demandas[indices[0]]);
        
        // BUG [1] FIX: Corrida individual é 1 trecho de DESLOCAMENTO (TRANS_PATH) 
        Path* p = new Path(start, end, TRANS_PATH, dist, time); 
        r->addPath(p);
        r->setTime_P(time);
        delete start;
        delete end;
    } else {
        // Rota básica (P0...Pk-1, D0...Dk-1) 
        Stop* stops[MAIN_MAX_PATHS];
        int numStops = 0;
        // Paradas de Coleta (P0...Pk-1)
        for (int i = 0; i < count; i++) {
            stops[numStops++] = new Stop(demandas[indices[i]]->getOriginX(), demandas[indices[i]]->getOriginY(), STOP_PICKUP, demandas[indices[i]]);
        }
        // Paradas de Entrega (D0...Dk-1)
        for (int i = 0; i < count; i++) {
            stops[numStops++] = new Stop(demandas[indices[i]]->getDestX(), demandas[indices[i]]->getDestY(), STOP_DROPOFF, demandas[indices[i]]);
        }

        double totalDist = 0.0, totalTime = 0.0;

        // numStops = 2 * count
        // O loop 'i' vai de 1 a (2*count - 1)
        for (int i = 1; i < numStops; i++) {
            double dist = dist2d(stops[i-1]->getX(), stops[i-1]->getY(), stops[i]->getX(), stops[i]->getY());
            double time = dist / gamma;
            
            // BUG [2] FIX: Identificar os 3 tipos de trecho 
            Path_Type type;
            if (i < count) {
                // Trechos 1 a (count-1): (P0->P1), ... (P(k-2)->P(k-1))
                type = PICKUP_PATH; // Coleta
            } else if (i == count) {
                // Trecho 'count': (P(k-1) -> D0)
                type = TRANS_PATH; // Deslocamento
            } else {
                // Trechos (count+1) a (2*count-1): (D0->D1), ...
                type = DROPOFF_PATH; // Entrega
            }

            Path* p = new Path(stops[i-1], stops[i], type, dist, time);
            r->addPath(p);
            totalDist += dist;
            totalTime += time;
        }
        r->setTime_P(totalTime);
        for (int i = 0; i < numStops; i++) delete stops[i];
    }
    r->C_eficiency();
    return r;
}

int main(int argc, char *argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    istream* input_stream = &cin;
    ifstream arquivo;
    
    if (argc == 2) {
        arquivo.open(argv[1]);
        if (!arquivo.is_open()) return 1;
        input_stream = &arquivo;
    }

    int eta;
    double gamma, delta, alpha, beta, lambda;
    int n;
    
    *input_stream >> eta >> gamma >> delta >> alpha >> beta >> lambda;
    *input_stream >> n;

    Demand* demandas[MAIN_MAX_DEMANDS];
    bool used[MAIN_MAX_DEMANDS] = {false};

    for (int i = 0; i < n; i++) {
        int id;
        double s_time, originX, originY, destX, destY;
        *input_stream >> id >> s_time >> originX >> originY >> destX >> destY;
        demandas[i] = new Demand(id, s_time, originX, originY, destX, destY);
    }
    
    if (arquivo.is_open()) arquivo.close();

    Run* runs[MAIN_MAX_RUNS];
    int numRuns = 0;

    for (int i = 0; i < n; i++) {
        if (used[i]) continue;
        
        Demand* currentGroup[MAIN_MAX_RUN_DEMANDS];
        int groupIndices[MAIN_MAX_RUN_DEMANDS];
        int groupSize = 0;
        
        currentGroup[groupSize] = demandas[i];
        groupIndices[groupSize] = i;
        groupSize++;
        used[i] = true;
        
    // [CORREÇÃO 1: Lógica de Agrupamento da Versão Básica]
    // Substitua o loop 'agressivo' por este (lógica "parar na primeira falha")

    for (int j = i + 1; j < n && groupSize < eta; j++) {
            if (used[j]) continue;

            if (demandas[j]->getS_time() - demandas[i]->getS_time() >= delta) {
                break;
            }

            // Critério 2: Alpha, Beta e Delta (PDF [cite: 40])
            if (!canCombineWithGroup(demandas[j], currentGroup, groupSize, delta, alpha, beta)) {
                break;
            }

            // Se passou nos critérios de tempo/distância, testamos a eficiência (Lambda)
            // Adicionamos temporariamente o índice de 'j' para calcular a rota
            groupIndices[groupSize] = j;

            // Usamos a função de rota BÁSICA (Corrigida na Seção 2)
            double actualDist = calculateActualRouteDistance(demandas, groupIndices, groupSize + 1);

            double individualDist = 0.0;
            for (int k = 0; k < groupSize + 1; k++) {
                individualDist += dist2d(demandas[groupIndices[k]]->getOriginX(),
                                        demandas[groupIndices[k]]->getOriginY(),
                                        demandas[groupIndices[k]]->getDestX(),
                                        demandas[groupIndices[k]]->getDestY());
            }
            
            double efficiency = (actualDist > 1e-9) ? individualDist / actualDist : 0.0;

            // Critério 3: Lambda (PDF [cite: 43])
            if (efficiency >= lambda) {
                // SUCESSO: A eficiência é boa.
                // Adicionamos permanentemente a demanda ao grupo.
                currentGroup[groupSize] = demandas[j];
                groupSize++;
                used[j] = true;
            } else {
                // FALHA: A eficiência não foi atingida.
                // PDF[cite: 43]: "remova ci de C ... interrompa a avaliação e conclua a definição de r."
                // Não precisamos "remover" (pois só mexemos em 'groupIndices'),
                // mas devemos parar de procurar.
                break;
            }
        }

        // O restante do loop 'i' continua (cria a corrida, etc.)
        // Usamos a função de criação de rota BÁSICA (Corrigida na Seção 3)
        Run* newRun = createOptimizedRun(demandas, groupIndices, groupSize, gamma);
        runs[numRuns++] = newRun;
    }

    Scaler escalonador;
    escalonador.Initialize();

    for (int i = 0; i < numRuns; i++) {
        Run* r = runs[i];
        Demand* firstDemand = r->getD(0);
        double startTime = firstDemand->getS_time();
        double endTime = startTime + r->getTime_P();
        Event* e = new Event(endTime, END_RUN, nullptr, firstDemand, r);
        escalonador.Insert(e);
    }

    while (true) {
        Event* e = escalonador.RemoveNextEvent();
        if (e == nullptr) break;

        if (e->getType() == END_RUN) {
            Run* r = e->getRun();
            
            double totalDist = 0.0;
            for (int j = 0; j < r->getNumPaths(); j++) {
                totalDist += r->getPath(j)->getDistance();
            }
            
            Stop* stops[MAIN_MAX_PATHS];
            int numStops = 0;
            getRunStops(r, stops, numStops);
            
            cout << fixed << setprecision(2);
            cout << e->getTime() << " " << totalDist << " " << numStops;
            for (int j = 0; j < numStops; j++) {
                cout << " " << stops[j]->getX() << " " << stops[j]->getY();
            }
            cout << "\n";
            
            for (int j = 0; j < numStops; j++) {
                delete stops[j];
            }
        }
        delete e;
    }

    escalonador.Finish();

    for (int i = 0; i < numRuns; i++) delete runs[i];
    for (int i = 0; i < n; i++) delete demandas[i];

    return 0;
}