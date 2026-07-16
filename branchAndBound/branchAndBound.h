#pragma once

#include <vector>
#include <random>

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../solverGreedy/solverGreedy.h"
#include "../scatter/scatter.h"
#include "../grasp/grasp.h"

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
    // Umbral de tamano (nro de vertices) para elegir la heuristica de cota
    // inferior. Por debajo se usa Scatter: da la cota mas ajustada y a este
    // tamano es barato. Por encima se usa GRASP, que construye caminos largos
    // que llenan el presupuesto casi tan bien como Scatter pero ~8x mas barato
    // (el 2-opt de Scatter escala ~O(L^2..L^3) en el largo del camino y en
    // grafos grandes se dispara). Es calibrable; el driver real del costo es el
    // largo del camino (que depende de W), no solo la cantidad de vertices.
    static constexpr int UMBRAL_SCATTER = 100;
    // Construcciones GRASP (construir+refinar, se queda con la mejor) usadas
    // como cota inferior en grafos por encima de UMBRAL_SCATTER. 20 equilibra
    // calidad/tiempo: a 1000 vertices (caso de evaluacion) da ~97% del beneficio
    // de Scatter en ~1/4 del tiempo, y con menos varianza que valores mas bajos.
    static constexpr int ITER_GRASP_COTA = 20;

    const Grafo* grafo;
    std::mt19937* rng;     // no-dueño: se propaga a las heuristicas de cota inferior (Scatter/Grasp)
    long maxIteraciones;   // tope de expansiones de nodo
    long iteraciones;      // contador interno de expansiones

    std::vector<int> distInv;          // dijkstraInvertido(destino): min costo v->destino
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

    // Calcula el incumbente inicial eligiendo la heuristica segun el tamano del
    // grafo (ver UMBRAL_GRAFO_GRANDE). Devuelve el mejor camino factible.
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
