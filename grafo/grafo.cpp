#include "grafo.h"
#include <fstream>
#include <stdexcept>
#include <queue>
#include <sstream>
#include <climits>
#include <algorithm>

using namespace std;

Grafo::Grafo(int cantVert, int cantArist, int maxW)
    : cantVert(cantVert), cantArist(cantArist), maxW(maxW),
      listaAdy(cantVert) {}

Grafo Grafo::cargarDesdeArchivo(const string& filename) {
    ifstream file(filename);
    if (!file.is_open())
        throw runtime_error("No se pudo abrir el archivo: " + filename);

    int n, m, W;
    file >> n >> m >> W;

    Grafo g(n, m, W);

    int origen, destino, costo, beneficio;
    for (int k = 0; k < m; k++) {
        file >> origen >> destino >> costo >> beneficio;
        g.insertarArista(origen, destino, costo, beneficio);
    }
    return g;
}

void Grafo::insertarArista(int origen, int destino, int costo, int beneficio) {
    listaAdy[origen].push_back({destino, costo, beneficio});
}

vector<Nodo> Grafo::getVecinos(int idNodo) const {
    return listaAdy[idNodo];
}

int Grafo::getCantVert() const { return cantVert;}
int Grafo::getMaxW() const { return maxW;}
int Grafo::getNodoDestino() const { return cantVert - 1;}

vector<int> Grafo::dijkstra(int origen) const {
    vector<int> dist(cantVert, INT_MAX);
    priority_queue<pair<int,int>,
                        vector<pair<int,int>>,
                        greater<>> pq;
    dist[origen] = 0;
    pq.push({0, origen});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        if (d > dist[u]) continue;
        for (const Nodo& v : listaAdy[u]) {
            if (dist[u] + v.costo < dist[v.destino]) {
                dist[v.destino] = dist[u] + v.costo;
                pq.push({dist[v.destino], v.destino});
            }
        }
    }
    return dist;
}

vector<int> Grafo::dijkstraCamino(int origen, int destino) const {
    vector<int> dist(cantVert, INT_MAX);
    vector<int> prev(cantVert, -1); // predecesor de cada nodo
    priority_queue<pair<int,int>,
        vector<pair<int,int>>,
        greater<>> pq;

    dist[origen] = 0;
    pq.push({0, origen});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        if (d > dist[u]) continue;
        for (const Nodo& v : listaAdy[u]) {
            if (dist[u] + v.costo < dist[v.destino]) {
                dist[v.destino] = dist[u] + v.costo;
                prev[v.destino] = u; // guardas por dónde llegaste
                pq.push({dist[v.destino], v.destino});
            }
        }
    }

    // reconstruir camino desde destino hacia atrás
    vector<int> camino;
    if (dist[destino] == INT_MAX) return camino; // no hay camino

    for (int v = destino; v != -1; v = prev[v])
        camino.push_back(v);

    reverse(camino.begin(), camino.end());
    return camino; // [origen, ..., destino]
}

vector<int> Grafo::dijkstraInvertido(int destino) const {
    // Construye lista de adyacencia invertida y corre Dijkstra desde destino.
    // El resultado dist[v] = costo mínimo en peso de v hasta destino.
    vector<vector<Nodo>> listaInv(cantVert);
    for (int u = 0; u < cantVert; u++) {
        for (const Nodo& v : listaAdy[u]) {
            listaInv[v.destino].push_back({u, v.costo, v.beneficio});
        }
    }

    vector<int> dist(cantVert, INT_MAX);
    priority_queue<pair<int,int>,
                        vector<pair<int,int>>,
                        greater<>> pq;
    dist[destino] = 0;
    pq.push({0, destino});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        if (d > dist[u]) continue;
        for (const Nodo& v : listaInv[u]) {
            if (dist[u] + v.costo < dist[v.destino]) {
                dist[v.destino] = dist[u] + v.costo;
                pq.push({dist[v.destino], v.destino});
            }
        }
    }
    return dist;
}

float Grafo::getRatioMejorEntrada(int id) const {
    float mejor = -1.0f;
    for (int u = 0; u < cantVert; u++) {
        for (const Nodo& n : listaAdy[u]) {
            if (n.destino == id && n.costo > 0) {
                float r = (float)n.beneficio / n.costo;
                if (r > mejor) mejor = r;
            }
        }
    }
    return mejor;
}

Nodo Grafo::getArista(int a, int b) const{
    // retorna {costo, beneficio}, {0,0} en caso de que no exista arista
    vector<Nodo> vecinos = getVecinos(a);
    Nodo nodo;
    for (const Nodo& n : vecinos){
        if (n.destino == b){
            nodo = n;
            return nodo;
        }
    }
    // tiramos error ya que no se pueden devolver referencias nulas
    ostringstream msg;
    msg << "no se encontro arista desde " << a << " hasta "<< b << " error en metodo getArista";
    throw runtime_error(msg.str());
}

bool Grafo::existeArista(int origen, int final) const {
    vector<Nodo> vecinos = getVecinos(origen);

    for (const Nodo& n : vecinos){
        if (n.destino == final){
            return true;
        }
    }

    return false;
}


int Grafo::getIdNodoInicial() const{
    return 0;
}

int Grafo::getIdNodoFinal() const{
    return (int) listaAdy.size() - 1;
}

int Grafo::getPeso(int a, int b) const{
    vector<Nodo> vecinos = listaAdy[a];
    for (Nodo n : vecinos){
        if (n.destino == b){
            return n.costo;
        }
    }
    throw runtime_error("no se encontro nodo en get peso");
}
int Grafo::getBeneficio(int a, int b) const{
    vector<Nodo> vecinos = listaAdy[a];
    for (Nodo n : vecinos){
        if (n.destino == b){
            return n.beneficio;
        }
    }
    throw runtime_error("no se encontro nodo en get beneficio");
}