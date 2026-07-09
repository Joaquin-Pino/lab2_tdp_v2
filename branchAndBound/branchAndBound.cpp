#include "branchAndBound.h"

#include <climits>
#include <algorithm>

using namespace std;

SolverBranchAndBound::SolverBranchAndBound()
    : grafo(nullptr), maxIteraciones(0), iteraciones(0),
      totalBenefEntrada(0), mejorBeneficio(0) {}

SolverBranchAndBound::SolverBranchAndBound(const Grafo& grafo, long maxIteraciones)
    : grafo(&grafo), maxIteraciones(maxIteraciones), iteraciones(0),
      totalBenefEntrada(0), mejorBeneficio(0) {
    precomputarCotas();
}

void SolverBranchAndBound::precomputarCotas() {
    int n = grafo->getCantVert();
    maxBenefEntrada.assign(n, 0);
    for (int u = 0; u < n; u++) {
        for (const Nodo& arista : grafo->getVecinos(u)) {
            if (arista.beneficio > maxBenefEntrada[arista.destino]) {
                maxBenefEntrada[arista.destino] = arista.beneficio;
            }
        }
    }
    totalBenefEntrada = 0;
    for (int b : maxBenefEntrada) totalBenefEntrada += b;
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

Camino SolverBranchAndBound::calcularCotaInferior() {
    // El goloso es barato en cualquier tamano: siempre aporta un candidato base
    // y sirve de piso universal.
    SolverGreedy greedy(*grafo);
    evaluarCandidato(greedy.resolver());

    // Solo en grafos chicos/medianos se paga Scatter, que da una cota mucho mas
    // ajustada (combina soluciones) pero cuyo 2-opt se dispara con caminos
    // largos. En grafos grandes nos quedamos con la cota del goloso.
    if (grafo->getCantVert() <= UMBRAL_GRAFO_GRANDE) {
        Scatter scatter(*grafo);
        evaluarCandidato(scatter.resolver(5));
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

    for (const Nodo& arista : grafo->getVecinos(actual)) {
        int v = arista.destino;
        if (enCamino[v]) continue;
        if (distInv[v] == INT_MAX) continue;                 // v no alcanza al destino
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
    int destino = grafo->getIdNodoFinal();
    int origen = grafo->getIdNodoInicial();
    int n = grafo->getCantVert();

    distInv = grafo->dijkstraInvertido(destino);

    // 1) cota inferior con las heuristicas
    mejorCamino.clear();
    mejorBeneficio = 0;
    calcularCotaInferior();

    iteraciones = 0;

    // 2) busqueda exacta con iterative deepening
    for (int prof = 2; prof <= n && iteraciones < maxIteraciones; prof++) {
        vector<char> enCamino(n, 0);
        vector<int> caminoActual;
        enCamino[origen] = 1;
        caminoActual.push_back(origen);
        dfs(origen, 0, 0, totalBenefEntrada - maxBenefEntrada[origen],
            prof, caminoActual, enCamino);
    }

    return Camino(mejorCamino, *grafo);
}
