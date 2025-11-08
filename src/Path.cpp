#include "Path.h"

//Construtor
Path::Path(){
    startStop = nullptr;
    endStop = nullptr;
    type  = TRANS_PATH; //padrão de trecho
    distance = 0.0;
    travelTime = 0.0;
}

Path::Path(Stop* startStop, Stop* endStop, Path_Type type, double distance, double travelTime){
    //Pegar os dados de cada trecho
    this->startStop = startStop;
    this->endStop = endStop;
    this->type = type;
    this->distance = distance;
    this->travelTime = travelTime;
}

//Getters
Stop* Path::getStartStop() const {
    return startStop;
}
Stop* Path::getEndStop() const {
    return endStop;
}
Path_Type Path::getType() const {
    return type;
}
double Path::getDistance() const {
    return distance;
}
double Path::getTravelTime() const {
    return travelTime;
}

//Setters
void Path::setStartStop(Stop* s) {
    startStop = s;
}
void Path::setEndStop(Stop* s) {
    endStop = s;
}
void Path::setType(Path_Type t) {
    type = t;
}
void Path::setDistance(double dist) {
    distance = dist;
}
void Path::setTravelTime(double tt) {
    travelTime = tt;
}

//Destructor
Path::~Path() {
    // Path não é dona das Stops.
    // Apenas limpa as referências para evitar ponteiros pendentes.
    startStop = nullptr;
    endStop = nullptr;
}