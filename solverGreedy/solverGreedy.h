#pragma once
#include "../grafo/grafo.h"
#include "../camino/camino.h"


// Heuristica constructiva mas simple: en cada paso avanza al vecino no
// visitado de mejor razon beneficio/costo que entre en el peso restante,
// hasta llegar al destino o quedar sin candidatos. Sin backtracking ni
// reconsideracion: si se traba antes del destino, devuelve el camino parcial
// tal cual (no lo completa con dijkstra). Sirve como solucion base barata
// para el resto de los solvers (Kopt, Breakout, Grasp/Scatter la completan).
class SolverGreedy {

private:
    const Grafo* grafo;

    // Vecino no visitado, de mejor beneficio/costo, que entra en pesoDisponible.
    // -1 si ninguno cumple (fin de la construccion).
    int obtenerMejorNodo(int nodoActual, int pesoDisponible, const Camino& sol);
public:

    SolverGreedy(const Grafo& grafo);
    Camino resolver();

};