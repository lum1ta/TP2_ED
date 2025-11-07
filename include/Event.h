//O evento é o que é computado pelo escalonador e engloba o tempo do trecho,o que ocorre nele e qual demanda está associada
#ifndef EVENT_H
#define EVENT_H

#include <iostream>

using namespace std;

//Estrategia de robustez
//Avoid to include Path.h and Demand.h to prevent circular dependency
class Path;
class Demand;
class Run;

//Event types
enum Event_Type{
    NEW_DEMAND,
    START_RUN,
    ARRIVAL_STOP,
    PICKUP,
    DROPOFF,
    END_RUN
};

//event class that represents an ocurrance at an especific time
class Event{
    private:
        double time;            //time of the event
        Event_Type type;        //type of event

        //pointers to associated objects
        Path* pathPtr;          //pointer to the associated path
        Demand* demandPtr;      //pointer to the associated demand
        Run* runPtr;            //pointer to the associated run

    public:
        //Constructor
        Event();
        Event(double time, Event_Type type, Path* pathPtr, Demand* demandPtr, Run* runPtr);

        //Getters
        double getTime() const;
        Event_Type getType() const;
        Path* getPath() const;
        Demand* getDemand() const;
        Run* getRun() const;

        //Setters
        void setTime(double t);
        void setType(Event_Type type);
        void setPath(Path* p);
        void setDemand(Demand* d);
        void setRun(Run* r);

        //bool operator to compare events by time
        bool operator<(const Event& other) const;

        //Destructor
        ~Event();
};
#endif
