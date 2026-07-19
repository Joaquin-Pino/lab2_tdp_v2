#pragma once

#include <vector>
#include <random>
#include <string>

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../solverGreedy/solverGreedy.h"
#include "../grasp/grasp.h"
#include "../scatter/scatter.h"

// SolverBranchAndBound: el "mejor algoritmo" y orquestador.
//
// Estrategia:
//  1) Cota inferior (incumbente): corre Greedy (piso barato) y una heuristica
//     combinatoria elegida segun el tamano del grafo. El costo de 2-OPT (usado
//     como refinamiento tanto por Scatter como por GRASP) crece de forma
//     cuadratica con el largo del camino, asi que en grafos chicos se usa
//     Scatter (mejor heuristica disponible) acotado en combinaciones, y en
//     grafos grandes (>= UMBRAL_NODOS_SCATTER nodos) se usa GRASP multi-start,
//     que en esos tamanos da un incumbente equivalente a menor costo. Se queda
//     con el mejor camino factible; ese beneficio arranca como el mejor
//     conocido y sirve para podar el arbol desde el comienzo.
//  2) Busqueda exacta con DFS de una sola pasada (profundidad hasta n) + dos
//     podas:
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
    // Umbral de tamano (cantidad de nodos) que decide la heuristica de cota
    // inferior: por debajo se usa Scatter (mejor heuristica, pero su 2-OPT de
    // refinamiento es O(L^2) y se dispara en caminos largos); desde este tamano
    // en adelante se usa GRASP, que da un incumbente equivalente a menor costo
    // en esos grafos. Coincide con el UMBRAL_REFINE de Grasp (grafo/grasp.h)
    // por consistencia de criterio.
    static constexpr int UMBRAL_NODOS_SCATTER = 500;

    // Rondas de scatter-search (combinar todo el refSet + reseleccionar) y tope
    // determinista de combinaciones por corrida, usados solo cuando el grafo
    // esta debajo de UMBRAL_NODOS_SCATTER. 45 = una ronda de pares del refSet
    // (TAM_REFSET=10 -> C(10,2)); acota el trabajo sin depender del reloj.
    static constexpr int ITER_SCATTER_COTA = 5;
    static constexpr long MAX_COMB_SCATTER = 45;

    // Iteraciones (construir+refinar) del GRASP multi-start usado como cota
    // inferior en grafos >= UMBRAL_NODOS_SCATTER. 30 reproduce el incumbente
    // que da Scatter en esos tamanos (verificado sobre el benchmark) a menor
    // costo, ya que evita el 2-OPT repetido de las rondas de combinacion.
    static constexpr int ITER_GRASP_COTA = 30;

    const Grafo* grafo;
    std::mt19937* rng;     // no-dueño: se propaga a las heuristicas de cota inferior (Scatter/Grasp)
    long maxIteraciones;   // tope de expansiones de nodo
    long iteraciones;      // contador interno de expansiones

    std::vector<int> distInv;          // dijkstra(destino): min costo v->destino
    std::vector<int> distDesdeOrigen;  // dijkstra(origen): min costo origen->v
    // alcanzable[v]: existe algun camino origen->v->destino dentro del
    // presupuesto (distDesdeOrigen[v] + distInv[v] <= W). Los nodos que no lo
    // cumplen no pueden estar en ninguna solucion factible: se excluyen de la
    // cota superior y de la expansion del dfs.
    std::vector<char> alcanzable;
    std::vector<int> maxBenefEntrada;  // mejor beneficio de arista entrante por nodo (0 si no alcanzable)
    long totalBenefEntrada;            // suma de maxBenefEntrada (cota superior global)

    // Adyacencia con los vecinos ordenados por beneficio descendente: el dfs los
    // expande en ese orden para subir el incumbente antes y podar mas ramas.
    std::vector<std::vector<Nodo>> vecinosOrdenados;

    // mejor solucion conocida (incumbente / cota inferior)
    std::vector<int> mejorCamino;
    int mejorBeneficio;

    // Precomputa distInv/distDesdeOrigen, el filtro de alcanzabilidad y
    // maxBenefEntrada/totalBenefEntrada (ya filtrados) a partir de las aristas.
    void precomputarCotas();

    // Calcula el incumbente inicial con Greedy + (Scatter o GRASP, segun
    // nombreCotaInferior()). Devuelve el mejor camino factible.
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

    // Nombre de la heuristica que calcularCotaInferior() va a usar (o uso) como
    // cota inferior para este grafo: "Scatter" si getCantVert() < UMBRAL_NODOS_SCATTER,
    // "GRASP" en caso contrario. Depende solo del tamano del grafo, asi que puede
    // consultarse antes de llamar a resolver() (p.ej. para informarlo en el menu).
    std::string nombreCotaInferior() const;
};
