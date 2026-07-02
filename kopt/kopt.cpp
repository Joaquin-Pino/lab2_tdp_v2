#include "kopt.h"
#include "../solverGreedy/solverGreedy.h"
#include "../algoritmos/algoritmo.h"
#include <vector>
#include <numeric>

using namespace std;

Kopt::Kopt(): grafo(nullptr), k(0) {}

Kopt::Kopt(const Grafo& grafo, int k): grafo(&grafo), k(k) {}


// ===== K-OPT convencional (reordena nodos ya presentes en el camino) =====

Camino Kopt::resolver() {
    // construir solucion inicial con SolverGreedy
    SolverGreedy SolverGreedy(*grafo);
    // si el camino entregado por greedy no es completo (no llega al destino),
    Camino inicial = SolverGreedy.resolver();
    // completarlo antes de seguir (p.ej. dijkstraCamino desde el ultimo nodo
    if (!inicial.llegaFinal()){
        int idFinal = inicial.getUltimoNodo();
        vector<int> temp = grafo->dijkstraCamino(idFinal, grafo->getIdNodoFinal());
        inicial.concatenar(temp);
    }
    // hasta el destino); si tampoco asi se puede completar, no hay solucion
    // factible que mejorar
    return resolver(inicial);
    // delegar en resolver(inicial) con ese camino ya completo
    
}

Camino Kopt::resolver(const Camino& inicial) {
    Camino mejorActual = inicial;
    bool huboMejora = true;

    while (huboMejora) {
        huboMejora = false;
        std::vector<int> caminoActual = mejorActual.getCamino();
        int n = static_cast<int>(caminoActual.size());

        // punto 1: posiciones = INDICES 1..n-2, 
        std::vector<int> posicionesIdx(n - 2);
        std::iota(posicionesIdx.begin(), posicionesIdx.end(), 1); // genera la lista

        std::vector<std::vector<int>> combinaciones = Algoritmo::combinar(posicionesIdx, k);

        for (const std::vector<int>& combinacion : combinaciones) {
            if (huboMejora) break;

            // IDs de los nodos que hoy ocupan esas posiciones, en orden
            std::vector<int> idsActuales;
            idsActuales.reserve(combinacion.size());
            for (int idx : combinacion) {
                idsActuales.push_back(caminoActual[idx]);
            }

            // punto 2: permutar idsActuales
            std::vector<std::vector<int>> permutaciones = Algoritmo::permutar(idsActuales, k);

            for (const std::vector<int>& permutacion : permutaciones) {
                if (huboMejora) break;

                // punto 5: continue, no break — solo saltar este orden, no cortar la busqueda
                if (permutacion == idsActuales) continue;

                std::vector<int> candidatoCamino = caminoActual;
                for (size_t i = 0; i < combinacion.size(); ++i) {
                    candidatoCamino[combinacion[i]] = permutacion[i];
                }

                // punto 4: validar TODAS las aristas afectadas ANTES de calcular
                // peso/beneficio o decidir aceptacion. Este for termina completo
                // antes de seguir, no hay break prematuro mezclado con la aceptacion.
                bool aristasValidas = true;
                for (int idx : combinacion) {
                    int nodoPrev = candidatoCamino[idx - 1];
                    int nodoActual = candidatoCamino[idx];
                    int nodoSig = candidatoCamino[idx + 1];

                    if (!grafo->existeArista(nodoPrev, nodoActual) ||
                        !grafo->existeArista(nodoActual, nodoSig)) {
                        aristasValidas = false;
                        break;
                    }
                }
                if (!aristasValidas) continue; // descartar, siguiente permutacion

                // Recien aqui, con aristas ya validadas, se recalcula UNA sola vez
                double pesoTotal = 0.0;
                double beneficioTotal = 0.0;
                for (int i = 0; i < n - 1; ++i) {
                    pesoTotal += grafo->getPeso(candidatoCamino[i], candidatoCamino[i + 1]);
                    beneficioTotal += grafo->getBeneficio(candidatoCamino[i], candidatoCamino[i + 1]);
                }

                if (pesoTotal > grafo->getMaxW()) continue; // descartar, sobrepasa W

                if (beneficioTotal > mejorActual.getBeneficioTotal()) {
                    mejorActual = Camino(candidatoCamino, *grafo);
                    huboMejora = true;
                    break; // corta el for de permutaciones
                }
            }
        }
    }

    return mejorActual; // punto 3: faltaba el return
}


// ===== K-OPT con perturbacion (reemplaza nodos por otros no visitados) =====

Camino Kopt::perturbar(const Camino& inicial) {
    // posicionesPeores = las k posiciones interiores de inicial con peor
    // razon beneficio/costo (apoyarse en Camino::getRatioNodo)

    // candidatosNuevos = todos los nodos del grafo que no estan visitados en inicial
    // si hay menos de k candidatos nuevos disponibles: devolver inicial sin cambios
    // (no hay margen para perturbar)

    // permutaciones = Algoritmo::permutar(candidatosNuevos, k)

    // para cada permutacion:
    //   candidato = inicial con los nodos de posicionesPeores reemplazados,
    //   uno a uno y en el mismo orden, por los nodos de esta permutacion
    //
    //   validar que existan en el grafo todas las aristas afectadas por el
    //   reemplazo (mismo chequeo de huecos que en resolver)
    //   si falta alguna arista: descartar candidato, seguir con la siguiente permutacion
    //
    //   recalcular pesoTotal del candidato
    //   si pesoTotal > maxW: descartar candidato, seguir con la siguiente permutacion
    //
    //   si llega aqui el candidato es factible: aplicarlo y devolverlo de inmediato
    //   (no se exige beneficio mayor al de inicial: esta es la perturbacion,
    //   se acepta aunque empeore el beneficio; es una sola pasada, no hay
    //   loop "mientras no haya mejora")

    // si ninguna permutacion resulto factible: devolver inicial sin cambios
}
