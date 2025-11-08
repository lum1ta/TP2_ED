#include "Run.h"
#include <cmath>

//constructor
Run::Run(){
    id = -1; 
    numDemands =  0;
    numPaths = 0;
    time_passed = 0.0;
    dist_T = 0.0;
    eficiency = 100.0;

     for (int i = 0; i < MAX_DEMANDS; i++) {
        demands[i] = nullptr;
    }
    for (int i = 0; i < MAX_PATH; i++) {
        paths[i] = nullptr;
    }
}

Run::Run(int id){
    this->id = 0;
    numDemands = 0;
    numPaths = 0;
    time_passed = 0.0;
    dist_T = 0.0;
    eficiency = 100.0;

     for (int i = 0; i < MAX_DEMANDS; i++) {
        demands[i] = nullptr;
    }
    for (int i = 0; i < MAX_PATH; i++) {
        paths[i] = nullptr;
    }
}

//Demands 
void Run::addD(Demand* d) {
    if (numDemands < MAX_DEMANDS && d != nullptr) {
        demands[numDemands++] = d;
        d->setRun(this);  // vincula a demanda à corrida
    } else {
        std::cerr << "Erro: limite máximo de demandas atingido ou demanda nula." << std::endl;
    }
}

int Run::getNumD() const {
    return numDemands;
}

Demand* Run::getD(int i) const {
    if (i >= 0 && i < numDemands)
        return demands[i];
    return nullptr;
}

//Paths
void Run::addPath(Path* p) {
    if (numPaths < MAX_PATH && p != nullptr) {
        paths[numPaths++] = p;
    } else {
        std::cerr << "Erro: limite máximo de paths atingido ou path nulo." << std::endl;
    }
}

int Run::getNumPaths() const {
    return numPaths;
}

Path* Run::getPath(int i) const {
    if (i >= 0 && i < numPaths)
        return paths[i];
    return nullptr;
}

//Setters para as estatisticas
void Run::setTime_P(double t) {
    time_passed = t;
}
void Run::setEficiency(double e) {
    eficiency = e;
}
double Run::getTime_P() const {
    return time_passed;
}
double Run::getDist_T() const {
    return dist_T;
}
double Run::getEficiency() const {
    return eficiency;
}

//Destructor
Run::~Run(){
    // Não deleta os paths porque são gerenciados externamente (por outro TAD)
    // Apenas remove as referências internas
    for (int i = 0; i < numPaths; i++) {
        paths[i] = nullptr;
    }

    // As demandas também não são deletadas aqui (elas pertencem ao sistema principal)
    for (int i = 0; i < numDemands; i++) {
        demands[i] = nullptr;
    }
    numPaths = 0;
    numDemands = 0;
    time_passed = 0.0;
    dist_T = 0.0;
    eficiency = 100.0;
}
void Run::C_eficiency() {
    /*
        A eficiência de uma corrida pode ser calculada como:

        λ = (soma das distâncias diretas de cada demanda) / (distância total da corrida) * 100

        Esse cálculo mostra quanto da rota é "efetivamente útil".
        Uma corrida 100% eficiente significa que não há desvios ou compartilhamento ineficiente.
    */

    if (numDemands == 0 || dist_T == 0.0) {
        eficiency = 100.0;
        return;
    }

    double sum_direct = 0.0;

    for (int i = 0; i < numDemands; i++) {
        Demand* d = demands[i];
        if (d != nullptr) {
            double dx = d->getDestX() - d->getOriginX();
            double dy = d->getDestY() - d->getOriginY();
            sum_direct += sqrt(dx * dx + dy * dy);
        }
    }

    eficiency = (sum_direct / dist_T) * 100.0;

    // Evita que o valor passe de 100% por imprecisão numérica
    if (eficiency > 100.0)
        eficiency = 100.0;
}