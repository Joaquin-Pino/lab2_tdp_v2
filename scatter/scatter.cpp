#include "scatter.h"
#include "../solverGreedy/solverGreedy.h"
#include <algorithm>
#include <climits>

using namespace std;

Scatter::Scatter(): grafo(nullptr), rng(std::random_device{}()) {}

Scatter::Scatter(const Grafo& grafo): grafo(&grafo), rng(std::random_device{}()) {}

size_t Scatter::VectorIntHash::operator()(const vector<int>& v) const {
    size_t seed = v.size();
    for (int x : v) {
        seed ^= std::hash<int>{}(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

pair<vector<int>, vector<int>> Scatter::clavePar(const Camino& a, const Camino& b) {
    const vector<int>& va = a.getCamino();
    const vector<int>& vb = b.getCamino();
    return (va < vb) ? make_pair(va, vb) : make_pair(vb, va);
}

size_t Scatter::ParHash::operator()(const pair<vector<int>, vector<int>>& par) const {
    VectorIntHash h;
    return h(par.first) ^ (h(par.second) << 1);
}

Camino Scatter::resolver(int maxIter) {
    const int TAM_REFSET = 10;
    const int N_GEN      = 30;              // construcciones aleatorias iniciales
    const int N_REFINAR  = 2 * TAM_REFSET;  // cuantas de esas se refinan (colchon)
    const int MAX_DETOUR = 3;               // largo max de detour en el pulido final

    // --- 1. Poblacion inicial: construcciones + ancla golosa. Se refinan solo
    //        las N_REFINAR mejores en crudo (refinar es lo caro). ---
    vector<Camino> soluciones = generarSoluciones(N_GEN);
    sort(soluciones.begin(), soluciones.end(), greater<Camino>());
    size_t lim = min((size_t)N_REFINAR, soluciones.size());
    for (size_t i = 0; i < lim; ++i)
        soluciones[i] = refinar(soluciones[i]);
    sort(soluciones.begin(), soluciones.begin() + lim, greater<Camino>());

    //RefSet = las TAM_REFSET soluciones de mayor beneficio.
    vector<Camino> refSet(soluciones.begin(),
                          soluciones.begin() + min((size_t)TAM_REFSET, soluciones.size()));
    unordered_set<pair<vector<int>, vector<int>>, ParHash> usados;

    for (int iter = 0; iter < maxIter; ++iter) {
        vector<pair<Camino, Camino>> pares;
        for (size_t i = 0; i < refSet.size(); ++i)
            for (size_t j = i + 1; j < refSet.size(); ++j)
                if (!usados.count(clavePar(refSet[i], refSet[j])))
                    pares.push_back({refSet[i], refSet[j]});

        // refSet agotado (todos los pares ya combinados): conservar el mejor y
        // regenerar el resto para diversificar.
        if (pares.empty()) {
            Camino mejor = *max_element(refSet.begin(), refSet.end());
            refSet = generarSolucionesAleatorias(TAM_REFSET - 1);
            for (Camino& c : refSet) c = refinar(c);
            refSet.push_back(mejor);
            usados.clear();
            continue;
        }

        // los hijos compiten con el refSet actual; sobreviven las mejores.
        vector<Camino> candidatos = refSet;
        for (auto& [p1, p2] : pares) {
            candidatos.push_back(refinar(combinar(p1, p2)));
            usados.insert(clavePar(p1, p2));
        }
        sort(candidatos.begin(), candidatos.end(), greater<Camino>());
        candidatos.resize(min(candidatos.size(), (size_t)TAM_REFSET));
        refSet = candidatos;
    }

    // --- 4. Pulido final: solo al ganador se le corren detours largos (caros),
    //        para exprimir el presupuesto ocioso que haya quedado. ---
    Camino mejor = *max_element(refSet.begin(), refSet.end());
    return rellenarPresupuesto(mejor, MAX_DETOUR);
}

// 2-OPT reordena el camino y libera peso; rellenarPresupuesto usa ese peso
// libre para insertar nodos beneficiosos. Solo detours de 1 nodo aca: los mas
// largos (caros) se dejan para el pulido final del ganador.
Camino Scatter::refinar(const Camino& solucion) const {
    Kopt kopt(*grafo);
    Camino r = kopt.resolver(solucion, true, 2);
    return rellenarPresupuesto(r, 1);
}

bool Scatter::sinDuplicados(const vector<int>& mitad1,
                             const vector<int>& mitad2) const {
    
    // busqueda de duplicados en o(n)
    unordered_set<int> vistos(mitad1.begin(), mitad1.end());
    for (int nodo : mitad2) {
        if (vistos.count(nodo)) return false;
        vistos.insert(nodo);
    }
    return true;
}

pair<vector<int>, bool> Scatter::intentarUnion(const vector<int>& primera,
                                                const vector<int>& segunda) const {
    size_t corte1 = primera.size() / 2;
    size_t corte2 = segunda.size() / 2;

    vector<int> mitad1(primera.begin(), primera.begin() + corte1);
    vector<int> mitad2(segunda.begin() + corte2, segunda.end());

    if (mitad1.empty() || mitad2.empty()) {
        return {{}, false};
    }

    // Punto de unión debe existir como arista real del grafo
    if (!grafo->existeArista(mitad1.back(), mitad2.front())) {
        return {{}, false};
    }

    // ninguna mitad puede repetir nodos de la otra
    if (!sinDuplicados(mitad1, mitad2)) {
        return {{}, false};
    }

    vector<int> resultado = mitad1;
    resultado.insert(resultado.end(), mitad2.begin(), mitad2.end());

    double pesoTotal = 0.0;
    for (size_t i = 0; i + 1 < resultado.size(); ++i) {
        pesoTotal += grafo->getPeso(resultado[i], resultado[i + 1]);
    }
    if (pesoTotal > grafo->getMaxW()) {
        return {{}, false};
    }

    return {resultado, true};
}

Camino Scatter::combinar(const Camino& C1, const Camino& C2) const {
    const vector<int>& v1 = C1.getCamino();
    const vector<int>& v2 = C2.getCamino();

    // Opción A: primera mitad de C1 + segunda mitad de C2
    auto [caminoA, validoA] = intentarUnion(v1, v2);
    if (validoA) {
        return Camino(caminoA, *grafo);
    }

    // Opción B: primera mitad de C2 + segunda mitad de C1
    auto [caminoB, validoB] = intentarUnion(v2, v1);
    if (validoB) {
        return Camino(caminoB, *grafo);
    }

    // Ninguna combinación fue factible: se retorna el mejor padre
    return (C1.getBeneficioTotal() >= C2.getBeneficioTotal()) ? C1 : C2;
}


Camino Scatter::construccionAleatoria(const vector<int>& anclaCamino,
                                      const vector<int>& distInv) {
    const int vFin = grafo->getIdNodoFinal();

    // ancla demasiado corta: no hay tramo que perturbar, se devuelve tal cual
    if (anclaCamino.size() < 2) return Camino(anclaCamino, *grafo);

    // Se conserva un prefijo aleatorio del ancla (ruin-and-recreate): el prefijo
    // aporta la calidad de la golosa; la cola se reconstruye al azar mas abajo,
    // aportando la diversidad que la combinacion necesita.
    uniform_int_distribution<size_t> distCorte(1, anclaCamino.size() - 1);
    size_t corte = distCorte(rng);

    vector<int> camino(anclaCamino.begin(), anclaCamino.begin() + corte);
    unordered_set<int> enCamino(camino.begin(), camino.end());

    double pesoActual = 0.0;
    for (size_t i = 0; i + 1 < camino.size(); ++i)
        pesoActual += grafo->getPeso(camino[i], camino[i + 1]);
    int actual = camino.back();

    // Largo objetivo aleatorio: da diversidad de tamanos entre soluciones y
    // evita construir caminos de casi N nodos (que hacen carisimo al 2-OPT
    // posterior, cuya vecindad crece cuadraticamente con el largo).
    uniform_int_distribution<int> distLargo(1, grafo->getCantVert());
    int largoObjetivo = distLargo(rng);

    const double ALPHA = 0.3; // fraccion superior de candidatos que forma la RCL

    // Reconstruccion de la cola por extension GRASP: se agregan vecinos no
    // visitados (RCL por eficiencia) hasta el destino o el largo objetivo.
    while (actual != vFin && (int)camino.size() < largoObjetivo) {
        vector<CandidatoExtension> candidatos =
            candidatosExtension(actual, pesoActual, enCamino, distInv);
        if (candidatos.empty()) break;

        // RCL (estilo GRASP): se ordena por eficiencia (beneficio/peso) y se
        // elige al azar dentro del tramo superior. Mezcla intensificacion
        // (candidatos buenos) con diversificacion (eleccion aleatoria).
        sort(candidatos.begin(), candidatos.end(),
             [](const CandidatoExtension& a, const CandidatoExtension& b) {
                 return a.eficiencia > b.eficiencia;
             });
        size_t tamRcl = max((size_t)1, (size_t)(ALPHA * candidatos.size()));
        uniform_int_distribution<size_t> distIdx(0, tamRcl - 1);
        const CandidatoExtension& elegido = candidatos[distIdx(rng)];

        camino.push_back(elegido.nodo);
        enCamino.insert(elegido.nodo);
        pesoActual += elegido.costo;
        actual = elegido.nodo;
    }

    // Cerrar el camino hasta el destino con el camino mas corto disponible.
    return Camino(completarHastaDestino(camino, enCamino), *grafo);
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

Camino Scatter::solucionGreedy() const {
    SolverGreedy greedy(*grafo);
    Camino inicial = greedy.resolver();
    // si el goloso no llega al destino, se completa con el camino mas corto
    if (!inicial.llegaFinal()) {
        vector<int> cola = grafo->dijkstraCamino(inicial.getUltimoNodo(),
                                                 grafo->getIdNodoFinal());
        inicial.concatenar(cola);
    }
    return inicial;
}

void Scatter::buscarDetour(int actual, int destino, int maxNodos,
                           int pesoDisponible, int pesoBase, int benefBase,
                           int pesoAcum, int benefAcum,
                           const unordered_set<int>& enCamino,
                           vector<int>& actualNodos,
                           unordered_set<int>& usadosLocal,
                           Detour& mejor) const {
    for (const Nodo& vecino : grafo->getVecinos(actual)) {
        int w = vecino.destino;
        // no reutilizar nodos del camino (destino incluido) ni del propio detour
        if (enCamino.count(w) || usadosLocal.count(w)) continue;

        int nuevoPeso = pesoAcum + vecino.costo;
        if (nuevoPeso > pesoDisponible) continue; // el sub-camino ya no cabe
        int nuevoBenef = benefAcum + vecino.beneficio;

        actualNodos.push_back(w);
        usadosLocal.insert(w);

        // cerrar el detour w->destino (a->...->w->destino ya es un sub-camino
        // valido con >= 1 nodo nuevo)
        if (grafo->existeArista(w, destino)) {
            int pesoT = nuevoPeso + grafo->getPeso(w, destino);
            if (pesoT <= pesoDisponible) {
                int deltaBenef = (nuevoBenef + grafo->getBeneficio(w, destino)) - benefBase;
                if (deltaBenef > mejor.deltaBenef) {
                    mejor.nodos = actualNodos;
                    mejor.deltaBenef = deltaBenef;
                    mejor.deltaPeso = pesoT - pesoBase;
                }
            }
        }

        // profundizar mientras no se agote el largo maximo del segmento
        if ((int)actualNodos.size() < maxNodos) {
            buscarDetour(w, destino, maxNodos, pesoDisponible, pesoBase, benefBase,
                         nuevoPeso, nuevoBenef, enCamino, actualNodos, usadosLocal, mejor);
        }

        actualNodos.pop_back();
        usadosLocal.erase(w);
    }
}

Camino Scatter::rellenarPresupuesto(const Camino& solucion, int maxNodos) const {
    vector<int> camino = solucion.getCamino();
    unordered_set<int> enCamino(camino.begin(), camino.end());
    const int maxW = grafo->getMaxW();

    int pesoActual = 0;
    for (size_t i = 0; i + 1 < camino.size(); ++i) {
        pesoActual += grafo->getPeso(camino[i], camino[i + 1]);
    }

    // Se barre el camino aplicando, en cada arista, el mejor detour factible
    // apenas se encuentra. Un barrido inserta muchos detours; se repite mientras
    // alguno logre insertar. Es mas barato que reescanear todo el camino tras
    // cada insercion, porque el DFS -lo caro- se corre una vez por arista y barrido.
    bool huboInsercion = true;
    while (huboInsercion) {
        huboInsercion = false;

        for (size_t pos = 0; pos + 1 < camino.size(); ++pos) {
            int a = camino[pos];
            int b = camino[pos + 1];
            int pesoBase = grafo->getPeso(a, b);
            int benefBase = grafo->getBeneficio(a, b);
            // presupuesto disponible para el nuevo sub-camino a->...->b: lo que
            // sobra mas lo que se libera al quitar la arista a->b
            int pesoDisponible = maxW - pesoActual + pesoBase;
            if (pesoDisponible <= pesoBase) continue; // sin margen para agregar

            Detour mejor;
            vector<int> actualNodos;
            unordered_set<int> usadosLocal;
            buscarDetour(a, b, maxNodos, pesoDisponible, pesoBase, benefBase,
                         0, 0, enCamino, actualNodos, usadosLocal, mejor);

            if (!mejor.nodos.empty()) {
                camino.insert(camino.begin() + pos + 1,
                              mejor.nodos.begin(), mejor.nodos.end());
                for (int x : mejor.nodos) enCamino.insert(x);
                pesoActual += mejor.deltaPeso;
                huboInsercion = true;
                pos += mejor.nodos.size(); // saltar el segmento recien insertado
            }
        }
    }

    return Camino(camino, *grafo);
}

vector<Camino> Scatter::generarSolucionesAleatorias(int n){
    // distInv y el ancla se calculan UNA vez por poblacion (no por construccion):
    // ambos dependen solo del grafo, recalcularlos por solucion era puro costo.
    const vector<int> distInv = grafo->dijkstraInvertido(grafo->getIdNodoFinal());
    const vector<int> anclaCamino = solucionGreedy().getCamino();

    vector<Camino> soluciones;
    soluciones.reserve(n);
    for (int i = 0; i < n; ++i)
        soluciones.push_back(construccionAleatoria(anclaCamino, distInv));
    return soluciones;
}

vector<Camino> Scatter::generarSoluciones(int n){
    // Poblacion inicial = n perturbaciones del ancla + el ancla golosa misma.
    vector<Camino> soluciones = generarSolucionesAleatorias(n);
    soluciones.push_back(solucionGreedy());
    return soluciones;
}