#include "solverGreedy.h"

using namespace std;

SolverGreedy::SolverGreedy() : grafo(nullptr) {}

SolverGreedy::SolverGreedy(const Grafo& grafo) : grafo(&grafo) {}

int SolverGreedy::obtenerMejorNodo(int nodoActual, int pesoDisponible, const Camino& sol){
    float maxBeneficio = -1.0f;
    int mejorNodoId = -1;

    const vector<Nodo>& vecinos = grafo->getVecinos(nodoActual);
    for (const Nodo& nodo : vecinos){
        if (sol.nodoFueVisitado(nodo.destino)) continue;
        if (nodo.costo > pesoDisponible) continue;

        float razon = (float)nodo.beneficio / nodo.costo;
        if (razon > maxBeneficio) {
            maxBeneficio = razon;
            mejorNodoId = nodo.destino;
        }
    }
    return mejorNodoId;
}

Camino SolverGreedy::resolver(){
    Camino solucion({}, *grafo);

    int idActual = 0;
    int idDestino = grafo->getCantVert() - 1;
    int pesoRestante = grafo->getMaxW();

    while (idActual != idDestino){
        solucion.agregarNodo(idActual);

        int idSiguiente = obtenerMejorNodo(idActual, pesoRestante, solucion);

        if (idSiguiente == -1){
            return solucion;
        }

        Nodo arista = grafo->getArista(idActual, idSiguiente);
        pesoRestante -= arista.costo;
        idActual = idSiguiente;
    }
    
    solucion.agregarNodo(idActual);
    return solucion;
}
