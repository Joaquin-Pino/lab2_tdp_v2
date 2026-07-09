#include "scatter.h"
#include <algorithm>
#include <climits>
#include <string>

using namespace std;

Scatter::Scatter()
    : grafo(nullptr), rng(std::random_device{}()),
      maxNodosInsertar(3), umbralDensidad(0.6), grafoEsDenso(false), modo(MEJOR) {}

Scatter::Scatter(const Grafo& grafo, int maxNodosInsertar,
                 double umbralDensidad, ModoInsercion modo)
    : grafo(&grafo), rng(std::random_device{}()),
      maxNodosInsertar(maxNodosInsertar), umbralDensidad(umbralDensidad),
      modo(modo) {
    grafoEsDenso = densidadGrafo() >= umbralDensidad;
}

double Scatter::densidadGrafo() const {
    int n = grafo->getCantVert();
    if (n <= 1) return 0.0;
    long m = 0;
    for (int v = 0; v < n; ++v) m += (long)grafo->getVecinos(v).size();
    return (double)m / ((double)n * (n - 1));
}

// ---------------------------------------------------------------------------
// resolver: busqueda dispersa (scatter search)
// ---------------------------------------------------------------------------

Camino Scatter::resolver(int maxIter) {
    vector<Camino> poblacion = generarPoblacion(TAM_POBLACION);
    vector<Camino> refSet = seleccionarRefSet(poblacion, TAM_REFSET);

    for (int iter = 0; iter < maxIter; ++iter) {
        vector<Camino> candidatos = refSet; // refSet actual sobrevive a la seleccion

        for (size_t i = 0; i < refSet.size(); ++i) {
            for (size_t j = i + 1; j < refSet.size(); ++j) {
                candidatos.push_back(refinar(combinar(refSet[i], refSet[j])));
            }
        }

        vector<Camino> nuevo = seleccionarRefSet(candidatos, TAM_REFSET);
        if (mismosRefSet(nuevo, refSet)) break; // refSet estable -> converge
        refSet = nuevo;
    }

    // seleccionarRefSet deja el mejor primero.
    return refSet.front();
}

// ---------------------------------------------------------------------------
// Construccion GRASP
// ---------------------------------------------------------------------------

vector<Camino> Scatter::generarPoblacion(int n) {
    vector<int> distInv = grafo->dijkstraInvertido(grafo->getIdNodoFinal());
    vector<Camino> poblacion;
    poblacion.reserve(n);
    for (int i = 0; i < n; ++i) {
        poblacion.push_back(refinar(construirGrasp(distInv)));
    }
    return poblacion;
}

Camino Scatter::construirGrasp(const vector<int>& distInv) {
    const int inicio = grafo->getIdNodoInicial();
    const int fin = grafo->getIdNodoFinal();
    const double alpha = 0.3; // 0 = puro goloso, 1 = puro azar

    vector<int> camino = {inicio};
    unordered_set<int> enCamino = {inicio};
    double pesoActual = 0.0;

    while (camino.back() != fin) {
        auto candidatos = candidatosExtension(camino.back(), pesoActual,
                                              enCamino, distInv);
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

vector<Scatter::CandidatoExtension> Scatter::candidatosExtension(
        int actual, double pesoActual,
        const unordered_set<int>& enCamino,
        const vector<int>& distInv) const {
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

vector<int> Scatter::completarHastaDestino(vector<int> camino,
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

// ---------------------------------------------------------------------------
// Combinacion
// ---------------------------------------------------------------------------

Camino Scatter::combinar(const Camino& C1, const Camino& C2) const {
    if (!grafoEsDenso)
        return empalme(C1, C2);

    // Grafo denso: injerta subcadenas de un padre en el otro (trae nodos nuevos).
    const vector<int>& v1 = C1.getCamino();
    const vector<int>& v2 = C2.getCamino();
    const bool mejor = (modo == MEJOR);

    Camino best = (C1.getBeneficioTotal() >= C2.getBeneficioTotal()) ? C1 : C2;

    auto probar = [&](const vector<int>& base, const vector<int>& donante) {
        auto [resultado, ok] = insertarSubcadena(base, donante, mejor);
        if (!ok) return;
        Camino c(resultado, *grafo);
        if (c.getPesoTotal() <= grafo->getMaxW() &&
            c.getBeneficioTotal() > best.getBeneficioTotal())
            best = c;
    };
    probar(v1, v2);
    probar(v2, v1);

    return best;
}

pair<vector<int>, bool> Scatter::insertarSubcadena(const vector<int>& base,
                                                   const vector<int>& donante,
                                                   bool mejorFactible) const {
    const int maxW = grafo->getMaxW();
    unordered_set<int> enBase(base.begin(), base.end());

    int pesoBase = 0;
    for (size_t i = 0; i + 1 < base.size(); ++i)
        pesoBase += grafo->getPeso(base[i], base[i + 1]);

    bool encontrado = false;
    int mejorGanancia = INT_MIN;
    vector<int> mejorResultado;

    // Subcadenas contiguas de nodos interiores del donante (sus aristas internas
    // ya existen en el grafo). Largo 1..maxNodosInsertar.
    for (size_t p = 1; p + 1 < donante.size(); ++p) {
        int pesoInterno = 0;
        int beneficioInterno = 0;

        for (int L = 1; L <= maxNodosInsertar && p + L - 1 < donante.size() - 1; ++L) {
            int w1 = donante[p];
            int wk = donante[p + L - 1];

            if (enBase.count(wk)) break; // la subcadena repetiria un nodo de base
            if (L >= 2) { // acumula la arista wk-1 -> wk (contigua en donante)
                pesoInterno += grafo->getPeso(donante[p + L - 2], wk);
                beneficioInterno += grafo->getBeneficio(donante[p + L - 2], wk);
            }

            // Probar cada hueco (a,b) de base.
            for (size_t i = 0; i + 1 < base.size(); ++i) {
                int a = base[i];
                int b = base[i + 1];
                if (!grafo->existeArista(a, w1) || !grafo->existeArista(wk, b))
                    continue;

                int pesoNuevo = pesoBase - grafo->getPeso(a, b)
                              + grafo->getPeso(a, w1) + pesoInterno + grafo->getPeso(wk, b);
                if (pesoNuevo > maxW) continue;

                int ganancia = -grafo->getBeneficio(a, b)
                             + grafo->getBeneficio(a, w1) + beneficioInterno
                             + grafo->getBeneficio(wk, b);

                vector<int> resultado(base.begin(), base.begin() + i + 1);
                resultado.insert(resultado.end(), donante.begin() + p, donante.begin() + p + L);
                resultado.insert(resultado.end(), base.begin() + i + 1, base.end());

                if (!mejorFactible)
                    return {resultado, true};

                if (ganancia > mejorGanancia) {
                    mejorGanancia = ganancia;
                    mejorResultado = move(resultado);
                    encontrado = true;
                }
            }
        }
    }

    if (encontrado) return {mejorResultado, true};
    return {{}, false};
}

Camino Scatter::empalme(const Camino& C1, const Camino& C2) const {
    const vector<int>& v1 = C1.getCamino();
    const vector<int>& v2 = C2.getCamino();

    // Opcion A: primera mitad de C1 + segunda mitad de C2
    auto [caminoA, validoA] = intentarUnion(v1, v2);
    if (validoA) return Camino(caminoA, *grafo);

    // Opcion B: primera mitad de C2 + segunda mitad de C1
    auto [caminoB, validoB] = intentarUnion(v2, v1);
    if (validoB) return Camino(caminoB, *grafo);

    // Ninguna combinacion fue factible: se retorna el mejor padre.
    return (C1.getBeneficioTotal() >= C2.getBeneficioTotal()) ? C1 : C2;
}

pair<vector<int>, bool> Scatter::intentarUnion(const vector<int>& primera,
                                               const vector<int>& segunda) const {
    size_t corte1 = primera.size() / 2;
    size_t corte2 = segunda.size() / 2;

    vector<int> mitad1(primera.begin(), primera.begin() + corte1);
    vector<int> mitad2(segunda.begin() + corte2, segunda.end());

    if (mitad1.empty() || mitad2.empty()) return {{}, false};

    // Punto de union debe existir como arista real del grafo.
    if (!grafo->existeArista(mitad1.back(), mitad2.front())) return {{}, false};

    // Ninguna mitad puede repetir nodos de la otra.
    if (!sinDuplicados(mitad1, mitad2)) return {{}, false};

    vector<int> resultado = mitad1;
    resultado.insert(resultado.end(), mitad2.begin(), mitad2.end());

    double pesoTotal = 0.0;
    for (size_t i = 0; i + 1 < resultado.size(); ++i)
        pesoTotal += grafo->getPeso(resultado[i], resultado[i + 1]);
    if (pesoTotal > grafo->getMaxW()) return {{}, false};

    return {resultado, true};
}

bool Scatter::sinDuplicados(const vector<int>& mitad1,
                            const vector<int>& mitad2) const {
    unordered_set<int> vistos(mitad1.begin(), mitad1.end());
    for (int nodo : mitad2) {
        if (vistos.count(nodo)) return false;
        vistos.insert(nodo);
    }
    return true;
}

Camino Scatter::refinar(const Camino& solucion) const {
    Kopt kopt(*grafo);
    return kopt.resolver(solucion, true, 2);
}

// ---------------------------------------------------------------------------
// RefSet
// ---------------------------------------------------------------------------

vector<Camino> Scatter::seleccionarRefSet(vector<Camino> candidatos, int b) const {
    // Ordena por beneficio descendente.
    sort(candidatos.begin(), candidatos.end(),
         [](const Camino& x, const Camino& y) {
             return x.getBeneficioTotal() > y.getBeneficioTotal();
         });

    vector<Camino> refSet;
    unordered_set<string> vistos; // firma del camino, para deduplicar
    for (const Camino& c : candidatos) {
        if ((int)refSet.size() >= b) break;
        string firma;
        for (int nodo : c.getCamino()) { firma += to_string(nodo); firma += ','; }
        if (vistos.insert(firma).second) refSet.push_back(c);
    }
    return refSet;
}

bool Scatter::mismosRefSet(const vector<Camino>& a, const vector<Camino>& b) const {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].getCamino() != b[i].getCamino()) return false;
    return true;
}
