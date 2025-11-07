#ifndef SCALER_H
#define SCALER_H

//template do minheap aqui
#include "Event.h"
#include "MinHeap.h"
#include <iostream>

using namespace std;

#define MAX_EVENT 500  //high fixed limit


class Scaler {
    private:
        MinHeap* eventHeap; // Min-heap to manage events
        
    public:
        // Constructor
        Scaler();
    
        void Initialize();
        void Insert(Event* e);
        Event* RemoveNextEvent();
        void Finish();

        //Destructor
        ~Scaler();

};

#endif