#ifndef DEMAND_H
#define DEMAND_H

#include <string>
#include <iostream>
using namespace std;

//Possible states
enum StateDemand {
    DEMANDED,
    INDIVIDUAL,
    COMBINED,
    CONCLUIDED
};

//Avoid to include Run.h to prevent circular dependency - Estratégia de robustez
class Run;// I need to save the run data as asked

class Demand{
    private:
        int id;
        double S_time;
        double originX,originY;
        double destX,destY;
        StateDemand state;

        //Estatistics from execution
        double dist_T;
        double time_P;
        double eficiency;

        Run* runAss;
    public:
        //Constructor
        Demand();
        Demand(int id, double S_time, double originX, double originY, double destX, double destY);
    
        //Getters
        int getId() const;
        double getS_time() const;   
        double getOriginX() const;
        double getOriginY() const;
        double getDestX() const;
        double getDestY() const;
        StateDemand getState() const;
        Run* getRun() const; //Pointer to the associated run because I need its data

        void setState(StateDemand newState);
        void setRun(Run* runPtr);

        //Statistcs
        void setDist_T(double d);
        void setTime_P(double t);
        void setEficiency(double e);

        double getDist_T() const;
        double getTime_P() const;
        double getEficiency() const;

        //Estrategia de robustez
        //Destructor
        ~Demand();
};

#endif