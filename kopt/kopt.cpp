#include "kopt.h"
#include "../solverGreedy/solverGreedy.h"
#include "../algoritmos/algoritmo.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>

using namespace std;

Kopt::Kopt(const Grafo& grafo, std::mt19937& rng): grafo(&grafo), rng(&rng) {}

// wrapper para resolver(iniciaL, primeraMejora)
Camino Kopt::resolver(bool primeraMejora, int k) {
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
    return resolver(inicial, primeraMejora, k);
    // delegar en resolver(inicial, primeraMejora) con ese camino ya completo

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
Camino Kopt::resolver(const Camino& inicial, bool primeraMejora, int k, int maxPasadas) {
    Camino mejorActual = inicial;
    bool huboMejora = true;

    int pasadas = 0;
    while (huboMejora) {
        // Tope opcional de pasadas: en caminos largos cada pasada cuesta O(L^2)
        // (materializa C(L,2) combinaciones). -1 => sin tope (convergencia).
        if (maxPasadas >= 0 && pasadas >= maxPasadas) break;
        ++pasadas;
        huboMejora = false;
        vector<int> caminoActual = mejorActual.getCamino();
        int n = static_cast<int>(caminoActual.size());

        // mejor candidato de ESTA pasada (solo se usa en modo steepest descent)
        vector<int> mejorCandidatoPasada;
        double mejorBeneficioPasada = mejorActual.getBeneficioTotal();
        bool huboCandidatoPasada = false;

        // totales base del camino de esta pasada; los candidatos se evaluan
        // por DELTA sobre estos, sin recorrer el camino completo cada vez
        double pesoBase = mejorActual.getPesoTotal();
        double benefBase = mejorActual.getBeneficioTotal();

        // punto 1: posiciones = INDICES 1..n-2,
        vector<int> posicionesIdx(n - 2);
        iota(posicionesIdx.begin(), posicionesIdx.end(), 1); // genera la lista

        vector<vector<int>> combinaciones = Algoritmo::combinar(posicionesIdx, k);

        for (const vector<int>& combinacion : combinaciones) {
            if (primeraMejora && huboMejora) break;

            // IDs de los nodos que hoy ocupan esas posiciones, en orden
            vector<int> idsActuales;
            idsActuales.reserve(combinacion.size());
            for (int idx : combinacion) {
                idsActuales.push_back(caminoActual[idx]);
            }

            // aristas afectadas por reordenar estas posiciones: para cada
            // posicion idx cambian las aristas (idx-1,idx) y (idx,idx+1),
            // identificadas por su extremo izquierdo. Se deduplican (posiciones
            // contiguas comparten arista). Dependen solo de la combinacion, no
            // de la permutacion, asi que se calculan una vez por combinacion.
            set<int> afectados;
            for (int idx : combinacion) {
                afectados.insert(idx - 1);
                afectados.insert(idx);
            }
            double oldPeso = 0.0, oldBenef = 0.0;
            for (int L : afectados) {
                oldPeso  += grafo->getPeso(caminoActual[L], caminoActual[L + 1]);
                oldBenef += grafo->getBeneficio(caminoActual[L], caminoActual[L + 1]);
            }

            // punto 2: permutar idsActuales
            vector<vector<int>> permutaciones = Algoritmo::permutar(idsActuales, k);

            for (const vector<int>& permutacion : permutaciones) {
                if (primeraMejora && huboMejora) break;

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

                // Recien aqui, con aristas ya validadas, se evalua por DELTA:
                // solo las aristas afectadas cambian respecto del camino base.
                double newPeso = 0.0, newBenef = 0.0;
                for (int L : afectados) {
                    newPeso  += grafo->getPeso(candidatoCamino[L], candidatoCamino[L + 1]);
                    newBenef += grafo->getBeneficio(candidatoCamino[L], candidatoCamino[L + 1]);
                }
                double pesoTotal = pesoBase + (newPeso - oldPeso);
                double beneficioTotal = benefBase + (newBenef - oldBenef);

                if (pesoTotal > grafo->getMaxW()) continue; // descartar, sobrepasa W

                if (beneficioTotal > mejorActual.getBeneficioTotal()) {
                    if (primeraMejora) {
                        // first-improvement: se aplica de inmediato y se corta la pasada
                        mejorActual = Camino(candidatoCamino, *grafo);
                        huboMejora = true;
                        break; // corta el for de permutaciones
                    } else if (beneficioTotal > mejorBeneficioPasada) {
                        // steepest descent: solo se guarda si es mejor que lo visto
                        // hasta ahora EN ESTA PASADA; se aplica recien al terminar
                        // de recorrer toda la vecindad
                        mejorCandidatoPasada = candidatoCamino;
                        mejorBeneficioPasada = beneficioTotal;
                        huboCandidatoPasada = true;
                    }
                }
            }
        }

        if (!primeraMejora && huboCandidatoPasada) {
            mejorActual = Camino(mejorCandidatoPasada, *grafo);
            huboMejora = true;
        }
    }

    return mejorActual;
}

Camino Kopt::granSalto(const Camino& inicial, int kSalto) {
    vector<int> caminoActual = inicial.getCamino();
    int n = static_cast<int>(caminoActual.size());

    vector<int> posicionesIdx(n - 2);
    iota(posicionesIdx.begin(), posicionesIdx.end(), 1);

    // Se baraja el orden de las posiciones disponibles para no sesgar
    // siempre hacia las mismas primeras posiciones del vector.
    // (requiere <algorithm> para shuffle, y un generador de números aleatorios)
    shuffle(posicionesIdx.begin(), posicionesIdx.end(), *rng);

    // Se toman kSalto posiciones al azar (las primeras del vector ya mezclado)
    vector<int> combinacion(posicionesIdx.begin(),
                             posicionesIdx.begin() + min(kSalto, (int)posicionesIdx.size()));
    sort(combinacion.begin(), combinacion.end()); // mantener orden ascendente de índices

    vector<int> idsActuales;
    for (int idx : combinacion) idsActuales.push_back(caminoActual[idx]);

    // Se genera UNA sola permutación aleatoria de esos nodos (no todas las k! posibles)
    vector<int> permutacionAleatoria = idsActuales;
    shuffle(permutacionAleatoria.begin(), permutacionAleatoria.end(), *rng);

    vector<int> candidatoCamino = caminoActual;
    for (size_t i = 0; i < combinacion.size(); ++i) {
        candidatoCamino[combinacion[i]] = permutacionAleatoria[i];
    }

    // Se valida factibilidad, pero NO se exige mejora de beneficio
    if (!verificarAristas(combinacion, candidatoCamino)) {
        return inicial; // el salto propuesto no es factible en aristas, se descarta
    }

    double pesoTotal = 0.0;
    for (int i = 0; i < n - 1; ++i) {
        pesoTotal += grafo->getPeso(candidatoCamino[i], candidatoCamino[i + 1]);
    }
    if (pesoTotal > grafo->getMaxW()) {
        return inicial; // excede peso máximo, se descarta el salto
    }

    return Camino(candidatoCamino, *grafo);
}
