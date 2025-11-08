#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm> // apenas para stable_sort
#include <limits>

#include "Demand.h"
#include "Stop.h"
#include "Path.h"
#include "Run.h"
#include "Event.h"
#include "Scaler.h"

using namespace std;

// Função auxiliar de distância euclidiana
static double euclid(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2, dy = y1 - y2;
    return sqrt(dx*dx + dy*dy);
}

// Estrutura para armazenar informações de uma run
struct RunInfo {
    Run* run;
    Stop** stops;
    int numStops;
    double totalDistance;
    Path** paths;
    int numPaths;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // --- Leitura dos parâmetros ---
    int eta;
    double gamma, delta, alpha, beta, lambda;
    int numDemandas;
    if (!(cin >> eta)) { cerr << "Entrada inválida\n"; return 1; }
    cin >> gamma >> delta >> alpha >> beta >> lambda >> numDemandas;

    // --- Leitura das demandas ---
    Demand** demands = new Demand*[numDemandas];
    for (int i = 0; i < numDemandas; ++i) {
        int id;
        double time, ox, oy, dx, dy;
        cin >> id >> time >> ox >> oy >> dx >> dy;
        demands[i] = new Demand(id, time, ox, oy, dx, dy);
    }

    // Ordenar as demandas pelo tempo (ordenação estável)
    for (int i = 1; i < numDemandas; ++i) {
        Demand* key = demands[i];
        int j = i - 1;
        while (j >= 0 && demands[j]->getS_time() > key->getS_time()) {
            demands[j + 1] = demands[j];
            --j;
        }
        demands[j + 1] = key;
    }

    // --- Estruturas de controle de execução ---
    Run** runs = new Run*[numDemandas];
    RunInfo* runsInfo = new RunInfo[numDemandas];
    int numRuns = 0;

    // --- Construção das rotas ---
    for (int idx = 0; idx < numDemandas; ++idx) {
        Demand* c0 = demands[idx];
        if (c0->getState() != DEMANDED) continue;

        Demand** Cset = new Demand*[eta];
        int csize = 0;
        Cset[csize++] = c0;

        // Avalia candidatos
        for (int j = idx + 1; j < numDemandas; ++j) {
            if ((demands[j]->getS_time() - c0->getS_time()) >= delta) break;
            if (csize >= eta) break;

            Demand* cand = demands[j];
            bool ok = true;
            for (int k = 0; k < csize; ++k) {
                if (euclid(cand->getOriginX(), cand->getOriginY(), Cset[k]->getOriginX(), Cset[k]->getOriginY()) > alpha ||
                    euclid(cand->getDestX(), cand->getDestY(), Cset[k]->getDestX(), Cset[k]->getDestY()) > beta) {
                    ok = false;
                    break;
                }
            }
            if (!ok) break;

            Cset[csize++] = cand;

            // Constroi paradas temporárias
            int nStops = csize * 2;
            Stop** tmpStops = new Stop*[nStops];
            int p = 0;
            for (int a = 0; a < csize; ++a)
                tmpStops[p++] = new Stop(Cset[a]->getOriginX(), Cset[a]->getOriginY(), STOP_PICKUP, Cset[a]);
            for (int a = 0; a < csize; ++a)
                tmpStops[p++] = new Stop(Cset[a]->getDestX(), Cset[a]->getDestY(), STOP_DROPOFF, Cset[a]);

            double sharedDist = 0.0;
            for (int s = 0; s + 1 < nStops; ++s)
                sharedDist += euclid(tmpStops[s]->getX(), tmpStops[s]->getY(), tmpStops[s+1]->getX(), tmpStops[s+1]->getY());

            double sumDirect = 0.0;
            for (int a = 0; a < csize; ++a)
                sumDirect += euclid(Cset[a]->getOriginX(), Cset[a]->getOriginY(), Cset[a]->getDestX(), Cset[a]->getDestY());

            double eff = (sharedDist > 0.0) ? (sumDirect / sharedDist) * 100.0 : 100.0;

            for (int s = 0; s < nStops; ++s) delete tmpStops[s];
            delete[] tmpStops;

            if (eff < (lambda * 100.0)) {
                --csize;
                break;
            }
        }

        Run* run = new Run(numRuns);
        runs[numRuns] = run;

        for (int a = 0; a < csize; ++a) {
            run->addD(Cset[a]);
            Cset[a]->setState(COMBINED);
        }

        int nStops = csize * 2;
        Stop** stops = new Stop*[nStops];
        int pidx = 0;
        for (int a = 0; a < csize; ++a)
            stops[pidx++] = new Stop(Cset[a]->getOriginX(), Cset[a]->getOriginY(), STOP_PICKUP, Cset[a]);
        for (int a = 0; a < csize; ++a)
            stops[pidx++] = new Stop(Cset[a]->getDestX(), Cset[a]->getDestY(), STOP_DROPOFF, Cset[a]);

        int nPaths = (nStops > 0) ? (nStops - 1) : 0;
        Path** pathsArr = new Path*[max(1, nPaths)];
        double totalDist = 0.0;

        for (int s = 0; s < nPaths; ++s) {
            double d = euclid(stops[s]->getX(), stops[s]->getY(), stops[s+1]->getX(), stops[s+1]->getY());
            double tt = (gamma > 0.0) ? (d / gamma) : 0.0;
            Path* pth = new Path(stops[s], stops[s+1], TRANS_PATH, d, tt);
            run->addPath(pth);
            pathsArr[s] = pth;
            totalDist += d;
        }

        runsInfo[numRuns] = { run, stops, nStops, totalDist, pathsArr, nPaths };
        ++numRuns;
        delete[] Cset;
    }

    // --- Escalonador ---
    Scaler scheduler;
    scheduler.Initialize();

    for (int r = 0; r < numRuns; ++r) {
        Run* run = runsInfo[r].run;
        if (run->getNumD() <= 0) continue;
        Demand* firstD = run->getD(0);
        double startTime = firstD->getS_time();
        Event* e = new Event(startTime, ARRIVAL_STOP, nullptr, firstD, run);
        scheduler.Insert(e);
    }

    // --- Loop de eventos ---
    while (true) {
        Event* ev = scheduler.RemoveNextEvent();
        if (!ev) break;

        double now = ev->getTime();
        Run* run = ev->getRun();
        Path* p = ev->getPath();

        int runIndex = -1;
        for (int ri = 0; ri < numRuns; ++ri)
            if (runsInfo[ri].run == run) { runIndex = ri; break; }
        if (runIndex == -1) { delete ev; continue; }

        RunInfo& info = runsInfo[runIndex];

        if (!p) {
            if (info.numPaths > 0) {
                Path* next = info.paths[0];
                Event* ne = new Event(now + next->getTravelTime(), ARRIVAL_STOP, next, nullptr, run);
                scheduler.Insert(ne);
            } else {
                cout << fixed << setprecision(3)
                     << now << " " << info.totalDistance << " " << info.numStops;
                for (int s = 0; s < info.numStops; ++s)
                    cout << " " << info.stops[s]->getX() << " " << info.stops[s]->getY();
                cout << "\n";
            }
        } else {
            int idxPath = -1;
            for (int pi = 0; pi < info.numPaths; ++pi)
                if (info.paths[pi] == p) { idxPath = pi; break; }

            if (idxPath + 1 < info.numPaths) {
                Path* next = info.paths[idxPath + 1];
                Event* ne = new Event(now + next->getTravelTime(), ARRIVAL_STOP, next, nullptr, run);
                scheduler.Insert(ne);
            } else {
                cout << fixed << setprecision(3)
                     << now << " " << info.totalDistance << " " << info.numStops;
                for (int s = 0; s < info.numStops; ++s)
                    cout << " " << info.stops[s]->getX() << " " << info.stops[s]->getY();
                cout << "\n";

                double sumDirect = 0.0;
                for (int a = 0; a < run->getNumD(); ++a) {
                    Demand* dd = run->getD(a);
                    sumDirect += euclid(dd->getOriginX(), dd->getOriginY(),
                                        dd->getDestX(), dd->getDestY());
                }
                double eff = (info.totalDistance > 0.0)
                                 ? ((sumDirect / info.totalDistance) * 100.0)
                                 : 100.0;
                run->setEficiency(eff);
                run->setTime_P(now);
            }
        }
        delete ev;
    }

    scheduler.Finish();

    for (int r = 0; r < numRuns; ++r) {
        for (int p = 0; p < runsInfo[r].numPaths; ++p) delete runsInfo[r].paths[p];
        delete[] runsInfo[r].paths;
        for (int s = 0; s < runsInfo[r].numStops; ++s) delete runsInfo[r].stops[s];
        delete[] runsInfo[r].stops;
        delete runsInfo[r].run;
    }

    delete[] runsInfo;
    delete[] runs;

    for (int i = 0; i < numDemandas; ++i) delete demands[i];
    delete[] demands;

    return 0;
}
