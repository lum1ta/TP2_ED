#include "MinHeap.h"
#include <stdexcept>

//constructors
MinHeap::MinHeap(){
    size = 0;
}

//Comparison
//True if e1 < e2
bool MinHeap::compare(Event* e1, Event* e2){
    return e1->getTime() < e2->getTime();
}

//Insert
void MinHeap::Insert(Event* e){
    if(size >= MAX_HEAP){
        cerr << "Erro!Limite máximo atingido" << endl;
        return;
    }
    heapArray[size] = e;
    heapUp(size);
    size++;
}

//Remove
Event* MinHeap::RemoveMin(){
    if(isEmpty()){
        cerr << "Não há nada a excluir" <<endl;
        return nullptr;
    }
    Event* minEvent = heapArray[0];
    heapArray[0] = heapArray[size -1];
    size--;
    heapDown(0);
    return minEvent;
}

//Peek Min
Event* MinHeap::peekMin() const{
    if(isEmpty()){
        cerr << "Não há nada há excluir";
        return nullptr;
    }
    return heapArray[0];
}

//isEmpty
bool MinHeap::isEmpty() const{
    return size == 0;
}

//HeapUp
void MinHeap::heapUp(int index){
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (compare(heapArray[index], heapArray[parent])) {
            std::swap(heapArray[index], heapArray[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

//HeapDown
void MinHeap::heapDown(int index){
    while(true){
        int left = 2 *index +1; //maior
        int right = 2*index +2;//menor
        int smallest = index;

        if(left < size && compare(heapArray[left],heapArray[smallest])){
            smallest = left;
        }
        if(right < size && compare(heapArray[right],heapArray[smallest])){
            smallest = right;
        }
        if (smallest != index) {
            std::swap(heapArray[index], heapArray[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

//Clear 
void MinHeap::clear() {
    for (int i = 0; i < size; i++) {
        heapArray[i] = nullptr;
    }
    size = 0;
}

//Destructor
MinHeap::~MinHeap() {
    clear();
}




