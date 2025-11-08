#include "Event.h"
#include <iostream>

//Constructor
Event::Event(){
    double time = 0.0;            
    Event_Type type = NEW_DEMAND;       //vou colocar esse como padrão mas sei lá talvez mude
    Path* pathPtr = nullptr;         
    Demand* demandPtr = nullptr;      
    Run* runPtr = nullptr;            
}
Event::Event(double time, Event_Type type, Path* pathPtr, Demand* demandPtr, Run* runPt){
    this->time = time;
    this->type = type;
    this->pathPtr = pathPtr;
    this->demandPtr = demandPtr;
    this->runPtr = runPt;
}

//Getters
double Event::getTime() const {
    return time;
}
Event_Type Event::getType() const {
    return type;
}
Path* Event::getPath() const {
    return pathPtr;
}
Demand* Event::getDemand() const {
    return demandPtr;
}
Run* Event::getRun() const {
    return runPtr;
}

//Setters
void Event::setTime(double t) {
    time = t;
}
void Event::setType(Event_Type type) {
    this->type = type;
}

void Event::setPath(Path* p) {
    pathPtr = p;
}
void Event::setDemand(Demand* d) {
    demandPtr = d;
}
void Event::setRun(Run* r) {
    runPtr = r;
}

//bool operator
// Define a prioridade de eventos no min-heap (menor tempo tem prioridade)
bool Event::operator<(const Event& other) const {
    return this->time < other.time;
}

//Destructor
Event::~Event() {
    // Estratégia de robustez:
    // O evento *não é dono* de Path, Demand ou Run.
    // Ele apenas referencia essas estruturas,sem delete
    pathPtr = nullptr;
    demandPtr = nullptr;
    runPtr = nullptr;
}
