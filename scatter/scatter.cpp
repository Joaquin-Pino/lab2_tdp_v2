#include "scatter.h"
#include <algorithm>

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

    vector<Camino> soluciones = generarSoluciones(100);

    for (Camino& c : soluciones) {
        c = koptSolver.resolver(c, false, 2);
    }

    sort(soluciones.begin(), soluciones.end(), greater<Camino>());

    // solo por calidad, sin componente de diversidad, 
    //obtenemos los 10 mejores o la lista completa si hay menos de 10 soluciones
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

            vector<Camino> nuevas = generarSoluciones(TAM_REFSET - 1);
            for (Camino& c : nuevas) {
                c = koptSolver.resolver(c, false, 2);
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
                nuevo = koptSolver.resolver(nuevo, false, 2);
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

    return *max_element(refSet.begin(), refSet.end());
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
    int vIni = 0;
    int vFin = grafo->getIdNodoFinal();

    vector<int> camino = grafo->dijkstraCamino(vIni, vFin);

    double pesoActual = 0.0;
    for (size_t i = 0; i + 1 < camino.size(); ++i) {
        pesoActual += grafo->getPeso(camino[i], camino[i + 1]);
    }

    while (true) {
        vector<CandidatoInsercion> candidatos = generarCandidatos(camino, pesoActual);
        if (candidatos.empty()) break;

        // Sin filtro de calidad: se elige uniformemente entre TODOS los factibles
        uniform_int_distribution<size_t> distIdx(0, candidatos.size() - 1);
        const CandidatoInsercion& elegido = candidatos[distIdx(rng)];

        camino.insert(camino.begin() + elegido.posicion + 1, elegido.nodo);
        pesoActual += elegido.deltaPeso;
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

vector<Scatter::CandidatoInsercion> Scatter::generarCandidatos(const vector<int>& camino,
                                                                 double pesoActual) const {
    vector<CandidatoInsercion> candidatos;
    unordered_set<int> enCamino(camino.begin(), camino.end());

    for (size_t pos = 0; pos + 1 < camino.size(); ++pos) {
        int a = camino[pos];
        int b = camino[pos + 1];

        for (const Nodo& vecino : grafo->getVecinos(a)) {
            int u = vecino.destino;
            if (enCamino.count(u)) continue;
            if (!grafo->existeArista(u, b)) continue;

            double deltaPeso = grafo->getPeso(a, u) + grafo->getPeso(u, b)
                                - grafo->getPeso(a, b);
            if (pesoActual + deltaPeso > grafo->getMaxW()) continue;

            double deltaBeneficio = grafo->getBeneficio(a, u) + grafo->getBeneficio(u, b)
                                     - grafo->getBeneficio(a, b);

            double eficiencia = (deltaPeso > 0.0)
                                 ? (deltaBeneficio / deltaPeso)
                                 : deltaBeneficio;

            candidatos.push_back({u, pos, deltaBeneficio, deltaPeso, eficiencia});
        }
    }
    return candidatos;
}


 vector<Camino> Scatter::generarSoluciones(int n){
    return generarSolucionesAleatorias(n);
 }