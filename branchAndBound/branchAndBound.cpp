#include "branchAndBound.h"

#include <climits>
#include <algorithm>

using namespace std;

SolverBranchAndBound::SolverBranchAndBound(const Grafo& grafo, std::mt19937& rng, long maxIteraciones)
    : grafo(&grafo), rng(&rng), maxIteraciones(maxIteraciones), iteraciones(0),
      totalBenefEntrada(0), mejorBeneficio(0) {
    precomputarCotas();
}

void SolverBranchAndBound::precomputarCotas() {
    int n = grafo->getCantVert();
    int W = grafo->getMaxW();

    // Distancias minimas para el filtro de alcanzabilidad (B1) y la poda en peso
    // del dfs. El grafo es no dirigido, asi que dijkstra(destino) sirve como
    // costo v->destino (distInv) y dijkstra(origen) como costo origen->v.
    distInv = grafo->dijkstra(grafo->getIdNodoFinal());
    distDesdeOrigen = grafo->dijkstra(grafo->getIdNodoInicial());

    // alcanzable[v]: existe un recorrido origen->v->destino dentro del
    // presupuesto. distDesdeOrigen[v] + distInv[v] es cota inferior del peso de
    // cualquier camino que pase por v, asi que si excede W, v no puede estar en
    // ninguna solucion factible y se descarta por completo.
    alcanzable.assign(n, 0);
    for (int v = 0; v < n; v++) {
        if (distDesdeOrigen[v] == INT_MAX || distInv[v] == INT_MAX) continue;
        if ((long)distDesdeOrigen[v] + distInv[v] <= W) alcanzable[v] = 1;
    }

    // Mejor beneficio de arista entrante por nodo (cota superior admisible: en un
    // camino cada nodo aporta a lo sumo su mejor arista de entrada). Los nodos no
    // alcanzables quedan en 0, lo que aprieta la cota superior global desde el
    // arranque.
    maxBenefEntrada.assign(n, 0);
    for (int u = 0; u < n; u++) {
        for (const Nodo& arista : grafo->getVecinos(u)) {
            int v = arista.destino;
            if (!alcanzable[v]) continue;
            if (arista.beneficio > maxBenefEntrada[v]) maxBenefEntrada[v] = arista.beneficio;
        }
    }
    totalBenefEntrada = 0;
    for (int b : maxBenefEntrada) totalBenefEntrada += b;

    // Vecinos por nodo ordenados de mayor a menor beneficio: al expandirlos en
    // ese orden el dfs encuentra caminos buenos antes, sube mejorBeneficio y la
    // poda por cota superior corta mas ramas. Se ordena una sola vez.
    vecinosOrdenados.assign(n, {});
    for (int u = 0; u < n; u++) {
        vecinosOrdenados[u] = grafo->getVecinos(u);
        sort(vecinosOrdenados[u].begin(), vecinosOrdenados[u].end(),
             [](const Nodo& a, const Nodo& b) { return a.beneficio > b.beneficio; });
    }
}

bool SolverBranchAndBound::asegurarCompleto(Camino& c) const {
    if (c.getLargo() == 0) return false;

    if (!c.llegaFinal()) {
        int idUltimo = c.getUltimoNodo();
        vector<int> cola = grafo->dijkstraCamino(idUltimo, grafo->getIdNodoFinal());
        if (cola.empty()) return false;      // no hay forma de cerrar el camino
        c.concatenar(cola);
    }

    return c.esCaminoCompleto() && c.getPesoTotal() <= grafo->getMaxW();
}

void SolverBranchAndBound::evaluarCandidato(Camino c) {
    if (!asegurarCompleto(c)) return;
    if (c.getBeneficioTotal() > mejorBeneficio) {
        mejorBeneficio = c.getBeneficioTotal();
        mejorCamino = c.getCamino();
    }
}

std::string SolverBranchAndBound::nombreCotaInferior() const {
    return (grafo->getCantVert() < UMBRAL_NODOS_SCATTER) ? "Scatter" : "GRASP";
}

Camino SolverBranchAndBound::calcularCotaInferior() {
    // El goloso es barato en cualquier tamano: siempre aporta un candidato base
    // y sirve de piso universal.
    SolverGreedy greedy(*grafo);
    evaluarCandidato(greedy.resolver());

    // Heuristica combinatoria de la cota, elegida segun el tamano del grafo: el
    // 2-OPT de refinamiento es O(L^2) por pasada, asi que en grafos chicos se
    // usa Scatter (mejor heuristica, acotada en combinaciones para no dispararse)
    // y en grafos grandes se usa GRASP multi-start, que da un incumbente
    // equivalente a menor costo en esos tamanos (verificado en el benchmark).
    if (grafo->getCantVert() < UMBRAL_NODOS_SCATTER) {
        Scatter scatter(*grafo, *rng);
        evaluarCandidato(scatter.resolver(ITER_SCATTER_COTA, MAX_COMB_SCATTER));
    } else {
        Grasp grasp(*grafo, *rng);
        evaluarCandidato(grasp.resolver(ITER_GRASP_COTA));
    }

    // Red de seguridad: si ninguna heuristica dio un camino completo, usar el
    // camino de menor costo de origen a destino.
    if (mejorCamino.empty()) {
        vector<int> corto = grafo->dijkstraCamino(grafo->getIdNodoInicial(),
                                                  grafo->getIdNodoFinal());
        if (!corto.empty()) evaluarCandidato(Camino(corto, *grafo));
    }

    return Camino(mejorCamino, *grafo);
}

void SolverBranchAndBound::dfs(int actual, int pesoAcum, int beneficioAcum,
                               long beneficioRestante, int limiteProfundidad,
                               vector<int>& caminoActual, vector<char>& enCamino) {
    if (iteraciones >= maxIteraciones) return;
    iteraciones++;

    int destino = grafo->getIdNodoFinal();
    int W = grafo->getMaxW();

    if (actual == destino) {
        if (beneficioAcum > mejorBeneficio) {
            mejorBeneficio = beneficioAcum;
            mejorCamino = caminoActual;
        }
        return;
    }

    // no expandir mas alla del limite de profundidad de esta pasada
    if ((int)caminoActual.size() >= limiteProfundidad) return;

    // poda por cota superior: ni sumando la mejor entrada de cada nodo libre se
    // supera el mejor conocido.
    if (beneficioAcum + beneficioRestante <= mejorBeneficio) return;

    for (const Nodo& arista : vecinosOrdenados[actual]) {
        int v = arista.destino;
        if (enCamino[v]) continue;
        if (!alcanzable[v]) continue;                        // no entra en ningun camino factible (B1)
        if (pesoAcum + arista.costo + distInv[v] > W) continue; // ec. 3: factibilidad en peso

        enCamino[v] = 1;
        caminoActual.push_back(v);
        dfs(v, pesoAcum + arista.costo, beneficioAcum + arista.beneficio,
            beneficioRestante - maxBenefEntrada[v], limiteProfundidad,
            caminoActual, enCamino);
        caminoActual.pop_back();
        enCamino[v] = 0;

        if (iteraciones >= maxIteraciones) return;
    }
}

Camino SolverBranchAndBound::resolver() {
    int origen = grafo->getIdNodoInicial();
    int n = grafo->getCantVert();

    // distInv/distDesdeOrigen/alcanzable ya los precomputo el constructor.

    // 1) cota inferior con las heuristicas
    mejorCamino.clear();
    mejorBeneficio = 0;
    calcularCotaInferior();

    iteraciones = 0;

    // 2) busqueda exacta con DFS de una sola pasada (profundidad hasta n). Se
    // probo iterative deepening, pero con el tope de iteraciones re-expandir los
    // niveles superficiales en cada profundidad desperdicia el presupuesto y no
    // llega a los caminos largos de alto beneficio; el DFS plano, con vecinos
    // ordenados por beneficio y sembrado con el incumbente heuristico, gasta
    // todo el presupuesto bajando hacia buenas soluciones. En el benchmark el
    // DFS plano igualo o mejoro el beneficio (gigante +2.3%, mil +6.6%) a igual
    // tiempo.
    {
        vector<char> enCamino(n, 0);
        vector<int> caminoActual;
        enCamino[origen] = 1;
        caminoActual.push_back(origen);
        dfs(origen, 0, 0, totalBenefEntrada - maxBenefEntrada[origen],
            n, caminoActual, enCamino);
    }

    return Camino(mejorCamino, *grafo);
}
