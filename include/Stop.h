#ifndef STOP_H
#define STOP_H

#include <string>
#include "Demand.h"

using namespace std;

enum Stop_Type {
    PICKUP,
    DROPOFF
};

class Stop{
    private:
        double x,y;
        Stop_Type type;
        Demand* demandPtr; //Pointer to the associated demand
    public:
        //Constructor
        Stop();
        Stop(double x, double y, Stop_Type type, Demand* demandPtr);

        //Getters
        double getX() const;
        double getY() const;
        Stop_Type getType() const;
        Demand* getDemand() const;

        //Setters
        void setX(double newX);
        void setY(double newY);
        void setType(Stop_Type newType);
        void setDemand(Demand* newDemandPtr);

        //Estrategia de robustez
        //Destructor
        ~Stop();
};
#endif

