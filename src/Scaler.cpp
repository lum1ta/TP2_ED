#include "Scaler.h"

//constructor
Scaler::Scaler() {
    eventHeap = nullptr;
}

//Initialize
// Inicializa o escalonador e cria a heap de eventos
void Scaler::Initialize() {
    if (eventHeap != nullptr) {
        delete eventHeap; // garante que não há vazamento de memória
    }
    eventHeap = new MinHeap();
    cout << "[Scaler] Inicializado com sucesso." << endl;
}

//Insert
// Insere um novo evento na heap
void Scaler::Insert(Event* e) {
    if (!eventHeap) {
        cerr << "[Scaler] Erro: heap não inicializada!" << endl;
        return;
    }
    eventHeap->Insert(e);
    cout << "[Scaler] Evento inserido (tempo = " << e->getTime() << ")." << endl;
}

//Remove next event
// Remove e retorna o próximo evento (menor tempo)
Event* Scaler::RemoveNextEvent() {
    if (!eventHeap || eventHeap->isEmpty()) {
        cerr << "[Scaler] Nenhum evento disponível para remoção." << endl;
        return nullptr;
    }
    Event* nextEvent = eventHeap->RemoveMin();
    cout << "[Scaler] Evento removido (tempo = " << nextEvent->getTime() << ")." << endl;
    return nextEvent;
}

//Finish
// Finaliza o escalonador limpando a heap
void Scaler::Finish() {
    if (eventHeap) {
        eventHeap->clear();
        delete eventHeap;
        eventHeap = nullptr;
        cout << "[Scaler] Finalizado e heap limpa." << endl;
    }
}

//Destructor
Scaler::~Scaler() {
    Finish();
}
