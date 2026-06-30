#pragma once
#include "../grafo/grafo.h"
#include "../camino/camino.h"


class SolverGreedy {

private: 
    const Grafo* grafo;

    int obtenerMejorNodo(int nodoActual, int pesoDisponible, const Camino& sol);
public:

    SolverGreedy();
    SolverGreedy(const Grafo& grafo);
    Camino resolver();

};