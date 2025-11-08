#include "Stop.h"
#include "Demand.h"
#include <string.h>

//Constructors
Stop::Stop(){
    x,y = 0.0;
    Stop_Type type = PICKUP; //padrao de  parada ser de embarque
    Demand* demandPtr = nullptr; 
}

Stop::Stop(double x, double y, Stop_Type type, Demand* demandPtr){
    this->x = x;
    this->y = y;
    this->type = type;
    this->demandPtr = demandPtr;
}

//Getters
double Stop::getX() const {
    return x;
}
double Stop::getY() const {
    return y;
}
Stop_Type Stop::getType() const {
    return type;
}
Demand* Stop::getDemand() const {
    return demandPtr;
}

//Setters
void Stop::setX(double newX) {
    x = newX;
}
void Stop::setY(double newY) {
    y = newY;
}
void Stop::setType(Stop_Type newType) {
    type = newType;
}
void Stop::setDemand(Demand* newDemandPtr) {
    demandPtr = newDemandPtr;
}

//Destructor
Stop::~Stop() {
    // Estratégia de robustez: o Stop não é dono da Demand.
    // Apenas remove a referência para evitar ponteiro pendente.
    demandPtr = nullptr;
}
