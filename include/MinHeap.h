#ifndef MINHEAP_H
#define MINHEAP_H

#include "Event.h"
#include <iostream>

#define MAX_HEAP 10000 //high fixed limit

class MinHeap {
    private:
        Event* heapArray[MAX_HEAP];
        int size;
        void heapUp(int index); // Helper function to maintain heap property after insertion
        void heapDown(int index); // Helper function to maintain heap property after removal
        static bool compare(Event* e1, Event* e2);  // Comparison function to compare two events by time

    public:
        //Constructor
        MinHeap();
        
        void Insert(Event* e); // Insert an event into the heap
        Event* RemoveMin();     // Remove and return the event with the minimum time
        Event* peekMin() const; // Return the event with the minimum time without removing it
        bool isEmpty() const; // Check if the heap is empty
        int getSize() const { return size; } // Get the current size of the heap

        void clear(); // Clear the heap

        //Destructor 
        ~MinHeap();
};
#endif