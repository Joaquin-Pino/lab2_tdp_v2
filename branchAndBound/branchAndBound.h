#pragma once

#include <vector>

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../scatter/scatter.h"

// SolverBranchAndBound: el "mejor algoritmo" y orquestador.
//
// Estrategia:
//  1) Cota inferior (incumbente): corre las heuristicas ya implementadas
//     (Greedy, 2-OPT/Kopt, Breakout, Scatter) y se queda con el mejor camino
//     factible. Ese beneficio arranca como el mejor conocido y sirve para
//     podar el arbol desde el comienzo.
//  2) Busqueda exacta con iterative deepening (profundidad creciente) + DFS
//     con dos podas:
//       - factibilidad en peso (ec. 3 del enunciado):
//           pesoAcum + costo(vl->v) + distInv[v] > W   => se descarta v
//         con distInv = dijkstraInvertido(destino), precomputado una vez.
//       - cota superior de beneficio: beneficioAcum + beneficioRestante <= mejor
//         donde beneficioRestante suma la mejor arista de entrada de cada nodo
//         aun no usado (sobreestimacion admisible).
//  3) Limite de iteraciones (expansiones): al alcanzarlo devuelve el mejor
//     camino encontrado hasta el momento.
class SolverBranchAndBound {
private:
    const Grafo* grafo;
    long maxIteraciones;   // tope de expansiones de nodo
    long iteraciones;      // contador interno de expansiones

    std::vector<int> distInv;          // dijkstraInvertido(destino): min costo v->destino
    std::vector<int> maxBenefEntrada;  // mejor beneficio de arista entrante por nodo
    long totalBenefEntrada;            // suma de maxBenefEntrada (cota superior global)

    // mejor solucion conocida (incumbente / cota inferior)
    std::vector<int> mejorCamino;
    int mejorBeneficio;

    // Precomputa maxBenefEntrada y totalBenefEntrada a partir de las aristas.
    void precomputarCotas();

    // Corre las heuristicas implementadas y devuelve el mejor camino factible.
    Camino calcularCotaInferior();

    // Si el camino no termina en el destino lo cierra con el camino mas corto.
    // Devuelve true si el resultado es completo y respeta el presupuesto.
    bool asegurarCompleto(Camino& c) const;

    // Considera 'c' como candidato a incumbente: lo completa y, si es factible
    // y mejora, actualiza mejorCamino/mejorBeneficio.
    void evaluarCandidato(Camino c);

    // DFS con profundidad acotada. 'actual' ya esta en caminoActual/enCamino y
    // los acumulados ya lo reflejan.
    void dfs(int actual, int pesoAcum, int beneficioAcum, long beneficioRestante,
             int limiteProfundidad, std::vector<int>& caminoActual,
             std::vector<char>& enCamino);

public:
    SolverBranchAndBound();
    SolverBranchAndBound(const Grafo& grafo, long maxIteraciones = 2000000);

    Camino resolver();
};
