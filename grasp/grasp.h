#pragma once

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include <vector>
#include <random>
#include <unordered_set>

// Grasp: metaheuristica constructiva (Greedy Randomized Adaptive Search
// Procedure).
//
// construir() arma un camino nodo a nodo: en cada paso arma una lista
// restringida de candidatos (RCL) con los vecinos no visitados cuya
// eficiencia (beneficio/costo) es >= peorEf + alpha*(mejorEf-peorEf), y elige
// uno al azar entre ellos. Con alpha=0 el corte queda en peorEf y la RCL
// incluye a todos los candidatos (maximo azar); con alpha=1 el corte queda en
// mejorEf y la RCL se reduce a los empatados en la mejor eficiencia (puro
// goloso). distInv (dijkstra(destino): costo minimo de cada nodo al destino,
// valido como cota invertida por ser el grafo no dirigido) se precalcula una
// vez en el constructor y se usa para descartar candidatos que dejarian el
// camino sin forma de llegar al destino dentro del presupuesto.
//
// Tambien sirve como metaheuristica standalone: resolver(maxIter) repite
// construccion + refinamiento 2-opt y se queda con el mejor beneficio.
class Grasp {
public:
    // rng compartido inyectado desde afuera; debe sobrevivir al Grasp.
    // umbralRefine: por encima de ese tamano refinar() omite el 2-opt (default =
    // valor calibrado previo). Se expone para que el Planner lo pueda ajustar.
    Grasp(const Grafo& grafo, std::mt19937& rng, double alpha = 0.3,
          int umbralRefine = 500);

    // Una construccion golosa aleatorizada: parte de [0] y extiende por la
    // RCL, cerrando al destino. Si la extension se traba antes de llegar,
    // cierra con el camino mas corto disponible; si eso tampoco alcanza el
    // presupuesto, cae al camino de costo minimo origen->destino.
    Camino construir();

    // n construcciones GRASP, cada una refinada con 2-opt. Pensada para
    // sembrar la poblacion inicial de metaheuristicas combinadoras (Scatter).
    std::vector<Camino> generarPoblacion(int n);

    // GRASP como solver standalone: construir()+refinar() maxIter veces,
    // se queda con el de mayor beneficio.
    Camino resolver(int maxIter);

private:
    const Grafo* grafo;
    std::mt19937* rng; // no-dueño: lo inyecta quien construye el solver
    double alpha; // 0 = puro goloso, 1 = puro azar

    std::vector<int> distInv; // dijkstra(destino): min costo v->destino

    // Por encima de este tamano refinar() omite el 2-opt: sobre caminos largos
    // es O(L^2) por pasada y no mejora lo que ya aporta rellenar() en construir().
    // Lo fija el ctor (default calibrado); el Planner lo puede ajustar.
    int umbralRefine;

    struct CandidatoExtension {
        int nodo;
        int costo;
        int beneficio;
        double eficiencia; // beneficio/costo de la arista actual->nodo
    };

    // Vecinos no visitados de 'actual' que aun dejan una completacion factible
    // en peso hasta el destino (segun distInv).
    std::vector<CandidatoExtension> candidatosExtension(
        int actual, double pesoActual,
        const std::unordered_set<int>& enCamino) const;

    // Cierra un camino parcial hasta el destino con el camino mas corto, sin
    // reutilizar nodos (retrocede el tramo extendido si hiciera falta).
    std::vector<int> completarHastaDestino(std::vector<int> camino,
                                           std::unordered_set<int> enCamino) const;

    // Insercion codiciosa de nodos: mientras exista un nodo no visitado x y un
    // hueco (a,b) del camino tal que reemplazar la arista a-b por a-x-b aumente
    // el beneficio sin pasarse de W, inserta el de mayor ganancia. Conserva los
    // extremos. En instancias donde el peso no ata (W grande) es lo que llena el
    // camino de nodos de alto beneficio; el 2-opt no puede porque no agrega nodos.
    std::vector<int> rellenar(std::vector<int> camino) const;

    // Refina una solucion con 2-opt (Kopt, first-improvement, k=2).
    Camino refinar(const Camino& solucion) const;
};
