#ifndef RUN_H
#define RUN_H

#include <iostream>
#include "Demand.h"
#include "Path.h"

using namespace std;

#define MAX_PATH 100     // high  limit
#define MAX_DEMANDS 4   // passengers per run

class Run {
    private:
    int id;                             // id run
    Demand* demands[MAX_DEMANDS];       // associated demands pointers
    int numDemands;                     // demands registered

    Path* paths[MAX_PATH];          // paths sequence
    int numPaths;                   // number of paths

    double time_passed;           // Total time run
    double dist_T;                // Total distance run
    double eficiency;             // Run eficiency

public:
    // Constructor and Destructor
    Run();
    Run(int id);
    ~Run();

    // Getters and Setters
    void addD(Demand* d);
    void remLastD();
    int getNumD() const;
    Demand* getD(int i) const;
    int getId() const;
    void setId(int newId);


    // Paths
    void addPath(Path* p);
    int getNumPaths() const;
    Path* getPath(int i) const;

    // Statistics
    void setTime_P(double dist);
    void setEficiency(double e);

    double getTime_P() const;
    double getDist_T() const;
    double getEficiency() const;

    // Eficiency 
    void C_eficiency();
};

#endif