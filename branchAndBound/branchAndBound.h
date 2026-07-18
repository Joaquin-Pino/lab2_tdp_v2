#pragma once

#include <vector>
#include <random>

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../solverGreedy/solverGreedy.h"
#include "../scatter/scatter.h"

// SolverBranchAndBound: el "mejor algoritmo" y orquestador.
//
// Estrategia:
//  1) Cota inferior (incumbente): corre Greedy (piso barato) y Scatter (la
//     mejor heuristica, limitada por MAX_COMB_SCATTER) y se queda con el mejor
//     camino factible. Ese beneficio arranca como el mejor conocido y sirve
//     para podar el arbol desde el comienzo.
//  2) Busqueda exacta con iterative deepening (profundidad creciente) + DFS
//     con dos podas:
//       - factibilidad en peso (ec. 3 del enunciado):
//           pesoAcum + costo(vl->v) + distInv[v] > W   => se descarta v
//         con distInv = dijkstra(destino), precomputado una vez (grafo no
//         dirigido: la distancia de v al destino es la misma que la de
//         destino a v, asi que un solo dijkstra(destino) sirve como cota
//         invertida para todos los nodos).
//       - cota superior de beneficio: beneficioAcum + beneficioRestante <= mejor
//         donde beneficioRestante suma la mejor arista de entrada de cada nodo
//         aun no usado (sobreestimacion admisible).
//  3) Limite de iteraciones (expansiones): al alcanzarlo devuelve el mejor
//     camino encontrado hasta el momento.
class SolverBranchAndBound {
private:
    // Scatter es la mejor heuristica disponible, asi que se usa como cota
    // inferior en todo tamano de grafo. Para que no se dispare en grafos grandes
    // (su 2-opt escala ~O(L^2..L^3) con el largo del camino) se la limita con un
    // tope determinista de combinaciones. 45 = una ronda de pares del refSet
    // (TAM_REFSET=10 -> C(10,2)); acota el trabajo en grafos grandes y casi no
    // muerde en los chicos, donde Scatter converge antes por refSet estable. Es
    // calibrable contra el benchmark (beneficio vs. tiempo en mil*/gigante).
    static constexpr long MAX_COMB_SCATTER = 45;
    // Rondas de scatter-search (combinar todo el refSet + reseleccionar).
    static constexpr int ITER_SCATTER_COTA = 5;

    const Grafo* grafo;
    std::mt19937* rng;     // no-dueño: se propaga a las heuristicas de cota inferior (Scatter/Grasp)
    long maxIteraciones;   // tope de expansiones de nodo
    long iteraciones;      // contador interno de expansiones

    std::vector<int> distInv;          // dijkstra(destino): min costo v->destino
    std::vector<int> maxBenefEntrada;  // mejor beneficio de arista entrante por nodo
    long totalBenefEntrada;            // suma de maxBenefEntrada (cota superior global)

    // Adyacencia con los vecinos ordenados por beneficio descendente: el dfs los
    // expande en ese orden para subir el incumbente antes y podar mas ramas.
    std::vector<std::vector<Nodo>> vecinosOrdenados;

    // mejor solucion conocida (incumbente / cota inferior)
    std::vector<int> mejorCamino;
    int mejorBeneficio;

    // Precomputa maxBenefEntrada y totalBenefEntrada a partir de las aristas.
    void precomputarCotas();

    // Calcula el incumbente inicial con Greedy + Scatter (limitado). Devuelve el
    // mejor camino factible.
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
    // rng compartido inyectado desde afuera; debe sobrevivir al solver. B&B no
    // consume aleatoriedad por si mismo, solo la propaga a la cota inferior.
    SolverBranchAndBound(const Grafo& grafo, std::mt19937& rng, long maxIteraciones = 2000000);

    Camino resolver();
};
