#include "Demand.h"
#include <cmath>

//Constructor
Demand::Demand() {
    id = -1; //A contagem do id pode começar de zero
    S_time = 0.0;
    originX = originY = 0.0;
    destX = destY = 0.0;
    state = DEMANDED; //estado padrão da demanda
    dist_T = 0.0;
    time_P = 0.0;
    eficiency = 1.0;   // eficiência padrão = 100%
    runAss = nullptr; //Nenhuma corrida associada ainda
}

Demand::Demand(int id,double S_time,double originX,double originY,double destX,double destY){
    this->id = id;
    this->S_time = S_time;
    this->originX = originX;
    this->originY = originY;
    this->destX = destX;
    this->destY = destY;
    this->state = DEMANDED;

    dist_T = 0.0;
    time_P = 0.0;
    eficiency = 100.0;   // eficiência padrão = 100%
    runAss = nullptr;

}

//Getters
int Demand::getId() const {return id;}
double Demand::getS_time() const {return S_time;}
double Demand::getOriginX() const {return originX;}
double Demand::getOriginY() const {return originY;}
double Demand::getDestX() const {return destX;}
double Demand::getDestY() const {return destY;}

StateDemand Demand::getState() const{return state;}
Run* Demand::getRun() const {return runAss;}

//Setters
void Demand::setState(StateDemand newState){
    state = newState;
}
void Demand::setRun(Run* runPtr){
    runAss = runPtr;
}

//Statiscs
void Demand::setDist_T(double d) {
    dist_T = d;
}
void Demand::setTime_P(double t) {
    time_P = t;
}
void Demand::setEficiency(double e) {
    eficiency = e;
}
double Demand::getDist_T() const {
    return dist_T;
}
double Demand::getTime_P() const {
    return time_P;
}
double Demand::getEficiency() const {
    return eficiency;
}

//Destructor
Demand::~Demand() {
}