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
    Kopt koptSolver(*grafo);
    const int TAM_REFSET = 10;
    const int N_GEN = 30;                  // se generan pocas soluciones...
    const int N_REFINAR = 2 * TAM_REFSET;  // ...y se refina solo el mejor tramo
    const int MAX_SEG = 3;                 // largo max de detour en insertarSegmentos

    vector<Camino> soluciones = generarSoluciones(N_GEN);

    // Ordenar por beneficio CRUDO y refinar con 2-OPT (first-improvement) solo
    // las N_REFINAR mejores, no las N_GEN: refinar es lo caro. Se refina el
    // doble del refSet como colchon, porque una solucion mediocre en crudo
    // puede escalar al refinarla.
    sort(soluciones.begin(), soluciones.end(), greater<Camino>());
    size_t lim = min((size_t)N_REFINAR, soluciones.size());
    for (size_t i = 0; i < lim; ++i) {
        // 2-OPT reordena y libera peso; repararEInsertar rellena el presupuesto
        // ocioso con nodos beneficiosos. Esto es lo que sube por encima del
        // optimo local del 2-OPT (incluye al ancla golosa, que tenia presupuesto
        // sin usar).
        soluciones[i] = koptSolver.resolver(soluciones[i], true, 2);
        soluciones[i] = repararEInsertar(soluciones[i]);
    }
    sort(soluciones.begin(), soluciones.begin() + lim, greater<Camino>());

    // obtenemos los 10 mejores (ya refinados) o la lista completa si hay menos
    vector<Camino> refSet(soluciones.begin(),
                           soluciones.begin() + min((size_t)TAM_REFSET, soluciones.size()));

    unordered_set<pair<vector<int>, vector<int>>, ParHash> usados;

    int iter = 0;
    while (iter < maxIter) {

        // Subset generation
        vector<pair<Camino, Camino>> pares;
        for (size_t i = 0; i < refSet.size(); ++i) {
            for (size_t j = i + 1; j < refSet.size(); ++j) {
                // si find no encuentra nada llega hasta el final de el set usados
                if (usados.find(clavePar(refSet[i], refSet[j])) == usados.end()) {
                    pares.push_back({refSet[i], refSet[j]});
                }
            }
        }

        if (pares.empty()) {
            // reconstruir : conservar solo el mejor, regenerar el resto
            // max_element retorna un iterador a un elemento del set
            Camino mejor = *max_element(refSet.begin(), refSet.end());

            vector<Camino> nuevas = generarSolucionesAleatorias(TAM_REFSET - 1);
            for (Camino& c : nuevas) {
                c = koptSolver.resolver(c, true, 2);
                c = repararEInsertar(c);
            }

            refSet.clear();
            refSet.push_back(mejor);
            refSet.insert(refSet.end(), nuevas.begin(), nuevas.end());

            usados.clear(); // RefSet es enteramente nuevo
        } else {
            //
            vector<Camino> pool;
            for (auto& par : pares) {
                Camino nuevo = combinar(par.first, par.second);
                nuevo = koptSolver.resolver(nuevo, true, 2);
                nuevo = repararEInsertar(nuevo);
                pool.push_back(nuevo);
                usados.insert(clavePar(par.first, par.second));
            }

            // Reference set update
            vector<Camino> todos = refSet;
            todos.insert(todos.end(), pool.begin(), pool.end());
            sort(todos.begin(), todos.end(), greater<Camino>());
            todos.resize(min(todos.size(), (size_t)TAM_REFSET));
            refSet = todos;
        }

        iter++;
    }

    // Pulido final del ganador: el operador de segmentos (caro) no se corre en
    // cada hijo, asi que se aplica una vez a la mejor solucion para exprimir el
    // presupuesto ocioso que haya quedado.
    Camino mejor = *max_element(refSet.begin(), refSet.end());
    mejor = repararEInsertar(mejor);
    mejor = insertarSegmentos(mejor, MAX_SEG);
    return mejor;
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


Camino Scatter::construccionAleatoria() {
    const int vFin = grafo->getIdNodoFinal();
    // costo minimo de cada nodo al destino: sirve para no extender por un nodo
    // desde el cual ya no se pueda cerrar el camino dentro del presupuesto.
    const vector<int> distInv = grafo->dijkstraInvertido(vFin);

    vector<int> camino;
    camino.push_back(grafo->getIdNodoInicial());
    unordered_set<int> enCamino;
    enCamino.insert(camino.front());

    double pesoActual = 0.0;
    int actual = camino.front();

    // Largo objetivo aleatorio: da diversidad de tamanos entre soluciones y
    // evita construir caminos de casi N nodos (que hacen carisimo al 2-OPT
    // posterior, cuya vecindad crece cuadraticamente con el largo).
    uniform_int_distribution<int> distLargo(1, grafo->getCantVert());
    int largoObjetivo = distLargo(rng);

    const double ALPHA = 0.3; // fraccion superior de candidatos que forma la RCL

    // Extension hacia adelante estilo GRASP: se agregan nodos al final del
    // camino (como el goloso), no por insercion en triangulos. Esto crece en
    // grafos dispersos, donde a lo largo del camino mas corto casi no existen
    // aristas u->b que habiliten la insercion clasica.
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

Camino Scatter::repararEInsertar(const Camino& solucion) const {
    vector<int> camino = solucion.getCamino();
    unordered_set<int> enCamino(camino.begin(), camino.end());
    const int maxW = grafo->getMaxW();

    // peso actual del camino (se recalcula una vez; luego se mantiene por delta)
    int pesoActual = 0;
    for (size_t i = 0; i + 1 < camino.size(); ++i) {
        pesoActual += grafo->getPeso(camino[i], camino[i + 1]);
    }

    // Best-improvement: en cada vuelta se aplica la insercion factible de mayor
    // ganancia de beneficio y se repite, porque cada insercion crea dos pares
    // nuevos donde pueden aparecer mas inserciones (efecto cascada).
    bool mejoro = true;
    while (mejoro) {
        mejoro = false;

        int mejorDeltaBenef = 0;   // solo se aceptan inserciones que aumentan beneficio
        int mejorDeltaPeso = 0;
        int mejorNodo = -1;
        size_t mejorPos = 0;

        for (size_t pos = 0; pos + 1 < camino.size(); ++pos) {
            int a = camino[pos];
            int b = camino[pos + 1];
            int pesoAB = grafo->getPeso(a, b);
            int benefAB = grafo->getBeneficio(a, b);

            for (const Nodo& vecino : grafo->getVecinos(a)) {
                int u = vecino.destino;
                if (enCamino.count(u)) continue;
                if (!grafo->existeArista(u, b)) continue;

                int deltaPeso = vecino.costo + grafo->getPeso(u, b) - pesoAB;
                if (pesoActual + deltaPeso > maxW) continue;

                int deltaBenef = vecino.beneficio + grafo->getBeneficio(u, b) - benefAB;
                if (deltaBenef > mejorDeltaBenef) {
                    mejorDeltaBenef = deltaBenef;
                    mejorDeltaPeso = deltaPeso;
                    mejorNodo = u;
                    mejorPos = pos;
                }
            }
        }

        if (mejorNodo != -1) {
            camino.insert(camino.begin() + mejorPos + 1, mejorNodo);
            enCamino.insert(mejorNodo);
            pesoActual += mejorDeltaPeso;
            mejoro = true;
        }
    }

    return Camino(camino, *grafo);
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

        // cerrar el detour w->destino (solo con >= 2 nodos: el caso de 1 nodo
        // ya lo cubre repararEInsertar)
        if ((int)actualNodos.size() >= 2 && grafo->existeArista(w, destino)) {
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

Camino Scatter::insertarSegmentos(const Camino& solucion, int maxNodos) const {
    vector<int> camino = solucion.getCamino();
    unordered_set<int> enCamino(camino.begin(), camino.end());
    const int maxW = grafo->getMaxW();

    int pesoActual = 0;
    for (size_t i = 0; i + 1 < camino.size(); ++i) {
        pesoActual += grafo->getPeso(camino[i], camino[i + 1]);
    }

    // Se barre el camino aplicando, en cada arista, el mejor detour factible
    // (best-improvement local) apenas se encuentra. Un barrido inserta muchos
    // segmentos; se repite mientras algun barrido logre insertar. Es mucho mas
    // barato que reescanear todo el camino tras cada insercion, porque el DFS
    // -que es lo caro- se corre una vez por arista y por barrido.
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
    vector<Camino> soluciones;
    soluciones.reserve(n);
    for (int i = 0; i < n; ++i) {
        soluciones.push_back(construccionAleatoria());
    }
    return soluciones;
}

vector<Camino> Scatter::generarSoluciones(int n){
    // Poblacion inicial = n construcciones aleatorias + el ancla golosa, que
    // garantiza que Scatter parta de al menos la calidad del goloso.
    vector<Camino> soluciones = generarSolucionesAleatorias(n);
    soluciones.push_back(solucionGreedy());
    return soluciones;
}