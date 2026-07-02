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
bool Kopt::verificarAristas(const vector<int>& combinacion, const vector<int>& candidatoCamino){
    for (int idx : combinacion) {
        int nodoPrev = candidatoCamino[idx - 1];
        int nodoActual = candidatoCamino[idx];
        int nodoSig = candidatoCamino[idx + 1];

        if (!grafo->existeArista(nodoPrev, nodoActual) ||
            !grafo->existeArista(nodoActual, nodoSig)) {
            return false;
        }
    }
    return true;
}
Camino Kopt::resolver(const Camino& inicial) {
    Camino mejorActual = inicial;
    bool huboMejora = true;

    while (huboMejora) {
        huboMejora = false;
        vector<int> caminoActual = mejorActual.getCamino();
        int n = static_cast<int>(caminoActual.size());

        // punto 1: posiciones = INDICES 1..n-2, 
        vector<int> posicionesIdx(n - 2);
        iota(posicionesIdx.begin(), posicionesIdx.end(), 1); // genera la lista

        vector<vector<int>> combinaciones = Algoritmo::combinar(posicionesIdx, k);

        for (const vector<int>& combinacion : combinaciones) {
            if (huboMejora) break;

            // IDs de los nodos que hoy ocupan esas posiciones, en orden
            vector<int> idsActuales;
            idsActuales.reserve(combinacion.size());
            for (int idx : combinacion) {
                idsActuales.push_back(caminoActual[idx]);
            }

            // punto 2: permutar idsActuales
            vector<vector<int>> permutaciones = Algoritmo::permutar(idsActuales, k);

            for (const vector<int>& permutacion : permutaciones) {
                if (huboMejora) break;

                // punto 5: continue, no break — solo saltar este orden, no cortar la busqueda
                if (permutacion == idsActuales) continue;

                vector<int> candidatoCamino = caminoActual;
                for (size_t i = 0; i < combinacion.size(); ++i) {
                    candidatoCamino[combinacion[i]] = permutacion[i];
                }

                // punto 4: validar TODAS las aristas afectadas ANTES de calcular
                // peso/beneficio o decidir aceptacion. Este for termina completo
                // antes de seguir, no hay break prematuro mezclado con la aceptacion.
                bool aristasValidas = verificarAristas(combinacion, candidatoCamino);
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
    vector<int> caminoActual = inicial.getCamino();

    int n = static_cast<int>(caminoActual.size());

    vector<int> posicionesIdx(n-2);
    iota(posicionesIdx.begin(), posicionesIdx.end(), 1);

    // se eligen las posiciones de los nodos que vamos a sacar del camino
    vector<vector<int>> combinaciones = Algoritmo::combinar(posicionesIdx, k);

    for (const vector<int>& combinacion : combinaciones){

        // IDs que hoy ocupan esas posiciones, para no aceptar un "reemplazo"
        // que deja los mismos nodos en el mismo orden
        vector<int> idsActuales;
        idsActuales.reserve(combinacion.size());
        for (int idx : combinacion) {
            idsActuales.push_back(caminoActual[idx]);
        }

        // candidatos: vecinos de los nodos que se sacan EN ESTA combinacion,
        // sin contar los que ya estan visitados en el camino
        unordered_set<int> idVecinos;
        for (int idx : combinacion){
            for (const Nodo& vecino : grafo->getVecinos(caminoActual[idx])){
                if (!inicial.nodoFueVisitado(vecino.destino)) {
                    idVecinos.insert(vecino.destino);
                }
            }
        }
        vector<int> candidatosNuevos(idVecinos.begin(), idVecinos.end());

        vector<vector<int>> permutaciones = Algoritmo::permutar(candidatosNuevos, k);

        for (const vector<int>& permutacion : permutaciones){
            if (permutacion == idsActuales) continue;

            vector<int> nuevoCamino = caminoActual;
            for (size_t i = 0; i < combinacion.size(); ++i) {
                nuevoCamino[combinacion[i]] = permutacion[i];
            }

            bool aristasValidas = verificarAristas(combinacion, nuevoCamino);
            if (!aristasValidas) continue; // seguimos con la siguiente permutacion

            double pesoTotal = 0.0;
            double beneficioTotal = 0.0;
            for (int i = 0; i < n - 1; ++i) {
                pesoTotal += grafo->getPeso(nuevoCamino[i], nuevoCamino[i + 1]);
                beneficioTotal += grafo->getBeneficio(nuevoCamino[i], nuevoCamino[i + 1]);
            }

            if (pesoTotal > grafo->getMaxW()) continue; // descartar, sobrepasa W

            // se acepta aunque empeore el beneficio: es el salto de Breakout
            return Camino(nuevoCamino, *grafo);
        }
    }

    return inicial; // si no se encontro nada, se retorna el inicial
}
