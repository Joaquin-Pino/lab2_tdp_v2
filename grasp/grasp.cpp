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

    // Con el camino ya cerrado en el destino, insertar nodos de alto beneficio
    // en los huecos mientras entren en el presupuesto. Clave cuando W no ata.
    if (camino.back() == fin)
        camino = rellenar(camino);

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

vector<int> Grasp::rellenar(vector<int> camino) const {
    const int W = grafo->getMaxW();
    unordered_set<int> enCamino(camino.begin(), camino.end());

    int peso = 0;
    for (size_t i = 0; i + 1 < camino.size(); ++i)
        peso += grafo->getPeso(camino[i], camino[i + 1]);

    // Best-improvement repetido: en cada vuelta busca la insercion de mayor
    // ganancia sobre todos los huecos y la aplica; corta cuando ninguna mejora.
    bool mejora = true;
    while (mejora) {
        mejora = false;
        int mejorGanancia = 0, mejorNodo = -1, mejorPos = -1, mejorDeltaPeso = 0;

        for (size_t i = 0; i + 1 < camino.size(); ++i) {
            int a = camino[i], b = camino[i + 1];
            int pesoAB  = grafo->getPeso(a, b);
            int benefAB = grafo->getBeneficio(a, b);

            // candidatos: vecinos de a que tambien lo sean de b (para armar
            // a-x-b) y que no esten ya en el camino.
            for (const Nodo& vecino : grafo->getVecinos(a)) {
                int x = vecino.destino;
                if (enCamino.count(x)) continue;
                if (!grafo->existeArista(x, b)) continue;

                int deltaPeso = vecino.costo + grafo->getPeso(x, b) - pesoAB;
                if (peso + deltaPeso > W) continue; // no entra en el presupuesto

                int ganancia = vecino.beneficio + grafo->getBeneficio(x, b) - benefAB;
                if (ganancia > mejorGanancia) {
                    mejorGanancia = ganancia;
                    mejorNodo = x;
                    mejorPos = (int)i;
                    mejorDeltaPeso = deltaPeso;
                }
            }
        }

        if (mejorNodo >= 0) {
            camino.insert(camino.begin() + mejorPos + 1, mejorNodo);
            enCamino.insert(mejorNodo);
            peso += mejorDeltaPeso;
            mejora = true;
        }
    }
    return camino;
}

Camino Grasp::refinar(const Camino& solucion) const {
    // En grafos grandes el 2-opt sobre caminos largos cuesta segundos por
    // construccion y casi no mejora frente a lo que ya deja rellenar(); se omite.
    if (grafo->getCantVert() > UMBRAL_REFINE) return solucion;
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
