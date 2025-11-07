#ifndef PATH_H
#define PATH_H

#include <iostream>
#include "Stop.h"

using namespace std;

enum Path_Type {
    PICKUP_PATH,
    DROPOFF_PATH,
    TRANS_PATH
};

class Path{
    private:
        Stop* startStop;
        Stop* endStop;
        Path_Type type;
        double distance;
        double travelTime;

    public:
        //Constructor
        Path();
        Path(Stop* startStop, Stop* endStop, Path_Type type, double distance, double travelTime);

        //Getters
        Stop* getStartStop() const;
        Stop* getEndStop() const;
        Path_Type getType() const;
        double getDistance() const;
        double getTravelTime() const;

        //Setters
        void setStartStop(Stop* s);
        void setEndStop(Stop* s);
        void setType(Path_Type type);
        void setDistance(double dist);
        void setTravelTime(double tt);

        //Destructor
        ~Path();

};
#endif