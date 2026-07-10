#include "grasp.h"
#include "../kopt/kopt.h"
#include <climits>
#include <algorithm>

using namespace std;

Grasp::Grasp() : grafo(nullptr), rng(std::random_device{}()), alpha(0.3) {}

Grasp::Grasp(const Grafo& grafo, double alpha)
    : grafo(&grafo), rng(std::random_device{}()), alpha(alpha) {
    distInv = grafo.dijkstraInvertido(grafo.getIdNodoFinal());
}

Camino Grasp::construir() {
    const int inicio = grafo->getIdNodoInicial();
    const int fin = grafo->getIdNodoFinal();

    vector<int> camino = {inicio};
    unordered_set<int> enCamino = {inicio};
    double pesoActual = 0.0;

    while (camino.back() != fin) {
        auto candidatos = candidatosExtension(camino.back(), pesoActual, enCamino);
        if (candidatos.empty()) break;

        double mejorEf = candidatos.front().eficiencia;
        double peorEf = candidatos.front().eficiencia;
        for (const auto& c : candidatos) {
            mejorEf = max(mejorEf, c.eficiencia);
            peorEf = min(peorEf, c.eficiencia);
        }
        double corte = peorEf + alpha * (mejorEf - peorEf);

        // RCL: candidatos suficientemente eficientes.
        vector<const CandidatoExtension*> rcl;
        for (const auto& c : candidatos)
            if (c.eficiencia >= corte) rcl.push_back(&c);

        uniform_int_distribution<size_t> dist(0, rcl.size() - 1);
        const CandidatoExtension* elegido = rcl[dist(rng)];

        camino.push_back(elegido->nodo);
        enCamino.insert(elegido->nodo);
        pesoActual += elegido->costo;
    }

    if (camino.back() != fin)
        camino = completarHastaDestino(camino, enCamino);

    Camino resultado(camino, *grafo);
    if (!camino.empty() && camino.back() == fin &&
        resultado.getPesoTotal() <= grafo->getMaxW())
        return resultado;

    // Fallback robusto: el camino de costo minimo siempre es simple y factible.
    return Camino(grafo->dijkstraCamino(inicio, fin), *grafo);
}

vector<Grasp::CandidatoExtension> Grasp::candidatosExtension(
        int actual, double pesoActual,
        const unordered_set<int>& enCamino) const {
    vector<CandidatoExtension> candidatos;
    const int maxW = grafo->getMaxW();

    for (const Nodo& vecino : grafo->getVecinos(actual)) {
        int u = vecino.destino;
        if (enCamino.count(u)) continue;
        if (distInv[u] == INT_MAX) continue; // u no alcanza el destino
        // debe quedar completacion factible en peso: pesoActual + arista + min(u->fin)
        if (pesoActual + vecino.costo + distInv[u] > maxW) continue;

        double eficiencia = (vecino.costo > 0)
                             ? (double)vecino.beneficio / vecino.costo
                             : vecino.beneficio;

        candidatos.push_back({u, vecino.costo, vecino.beneficio, eficiencia});
    }
    return candidatos;
}

vector<int> Grasp::completarHastaDestino(vector<int> camino,
                                         unordered_set<int> enCamino) const {
    const int vFin = grafo->getIdNodoFinal();

    while (true) {
        int desde = camino.back();
        if (desde == vFin) return camino; // ya termina en el destino

        vector<int> cola = grafo->dijkstraCamino(desde, vFin); // [desde, ..., vFin]

        // Si el camino mas corto reutiliza nodos ya visitados, no sirve: se
        // retrocede un nodo del tramo extendido y se reintenta. Como cada nodo
        // se agrego garantizando distInv finito, la peor completacion posible
        // es la del origen (camino simple), asi que el retroceso termina.
        bool disjunta = !cola.empty();
        for (size_t i = 1; i < cola.size() && disjunta; ++i) {
            if (enCamino.count(cola[i])) disjunta = false;
        }
        if (disjunta) {
            camino.insert(camino.end(), cola.begin() + 1, cola.end());
            return camino;
        }

        if (camino.size() <= 1) return camino; // no se pudo completar
        enCamino.erase(camino.back());
        camino.pop_back();
    }
}

Camino Grasp::refinar(const Camino& solucion) const {
    Kopt kopt(*grafo);
    return kopt.resolver(solucion, true, 2);
}

vector<Camino> Grasp::generarPoblacion(int n) {
    vector<Camino> poblacion;
    poblacion.reserve(n);
    for (int i = 0; i < n; ++i) {
        poblacion.push_back(refinar(construir()));
    }
    return poblacion;
}

Camino Grasp::resolver(int maxIter) {
    Camino mejor = refinar(construir());
    for (int iter = 1; iter < maxIter; ++iter) {
        Camino candidato = refinar(construir());
        if (candidato.getBeneficioTotal() > mejor.getBeneficioTotal())
            mejor = candidato;
    }
    return mejor;
}
