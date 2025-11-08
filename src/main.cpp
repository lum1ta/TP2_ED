#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "Scaler.h"
#include "Demand.h"
#include "Run.h"
#include "Path.h"
#include "Stop.h"
#include "Event.h"

using namespace std;

// Função auxiliar para calcular distância euclidiana
double calcDistance(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

// Função para criar uma corrida individual simples
Run* createIndividualRun(Demand* d, double vehicleSpeed) {
    // Cria os pontos de embarque e desembarque
    Stop* start = new Stop(d->getOriginX(), d->getOriginY(), STOP_PICKUP, d);
    Stop* end = new Stop(d->getDestX(), d->getDestY(), STOP_DROPOFF, d);

    // Calcula distância e tempo de viagem
    double distance = calcDistance(d->getOriginX(), d->getOriginY(), d->getDestX(), d->getDestY());
    double travelTime = distance / vehicleSpeed;

    // Cria o trecho (Path)
    Path* p = new Path(start, end, TRANS_PATH, distance, travelTime);

    // Cria a corrida (Run)
    Run* r = new Run(d->getId());
    r->addD(d);
    r->addPath(p);
    r->setTime_P(travelTime);
    r->setEficiency(1.0);  // 100%

    // Atualiza a demanda
    d->setRun(r);
    d->setState(CONCLUIDED);
    d->setDist_T(distance);
    d->setTime_P(travelTime);
    d->setEficiency(1.0);

    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Lê parâmetros globais
    double eta, gama, delta, alfa, beta, lambda;
    int numDemandas;
    cin >> eta >> gama >> delta >> alfa >> beta >> lambda >> numDemandas;

    vector<Demand*> demandas;
    Scaler escalonador;
    escalonador.Initialize();

    // Leitura das demandas
    for (int i = 0; i < numDemandas; i++) {
        int id;
        double tempo, ox, oy, dx, dy;
        cin >> id >> tempo >> ox >> oy >> dx >> dy;
        Demand* d = new Demand(id, tempo, ox, oy, dx, dy);
        demandas.push_back(d);

        // Agenda evento NEW_DEMAND
        Event* e = new Event(tempo, NEW_DEMAND, nullptr, d, nullptr);
        escalonador.Insert(e);
    }

    // Processa eventos
    while (true) {
        Event* e = escalonador.RemoveNextEvent();
        if (!e) break;

        switch (e->getType()) {
            case NEW_DEMAND: {
                Demand* d = e->getDemand();

                // Cria corrida individual
                Run* r = createIndividualRun(d, gama);

                // Cria evento END_RUN
                double endTime = d->getS_time() + r->getTime_P();
                Event* endEvent = new Event(endTime, END_RUN, nullptr, d, r);
                escalonador.Insert(endEvent);
                break;
            }

            case END_RUN: {
                Run* r = e->getRun();

                cout << fixed << setprecision(2)
                     << e->getTime() << " "
                     << r->getDist_T() << " "
                     << r->getNumPaths() << " ";

                // Imprime coordenadas das paradas
                for (int i = 0; i < r->getNumPaths(); i++) {
                    Path* p = r->getPath(i);
                    cout << fixed << setprecision(2)
                         << p->getStartStop()->getX() << " "
                         << p->getStartStop()->getY() << " "
                         << p->getEndStop()->getX() << " "
                         << p->getEndStop()->getY();
                    if (i != r->getNumPaths() - 1) cout << " ";
                }
                cout << "\n";
                break;
            }

            default:
                break;
        }

        delete e; // limpa evento processado
    }

    escalonador.Finish();

    // Libera memória
    for (Demand* d : demandas) delete d;

    return 0;
}
