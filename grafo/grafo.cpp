#include "grafo.h"
#include <fstream>
#include <stdexcept>
#include <queue>
#include <sstream>
#include <climits>
#include <algorithm>

using namespace std;

Grafo::Grafo(int cantVert, int cantArist, int maxW)
    : cantVert(cantVert), maxW(maxW),
      listaAdy(cantVert), ady(cantVert) {
    (void)cantArist;
}

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
    // Grafo NO DIRIGIDO: la arista se guarda en los dos extremos, de modo que
    // getVecinos/getArista/existeArista son simetricos por construccion.
    if (origen == destino) return; // un lazo nunca aparece en un camino simple

    // Idempotente: si la arista ya fue declarada (en cualquier sentido) se
    // conserva la primera. Los archivos grandes traen la misma arista repetida
    // y pares reciprocos con costo/beneficio distintos; sin esto listaAdy
    // tendria aristas paralelas que ady (un map) no puede representar, y
    // getVecinos dejaria de coincidir con getPeso/getBeneficio.
    if (existeArista(origen, destino)) return;

    listaAdy[origen].push_back({destino, costo, beneficio});
    ady[origen].emplace(destino, Nodo{destino, costo, beneficio});

    listaAdy[destino].push_back({origen, costo, beneficio});
    ady[destino].emplace(origen, Nodo{origen, costo, beneficio});
}

const vector<Nodo>& Grafo::getVecinos(int idNodo) const {
    return listaAdy[idNodo];
}

int Grafo::getCantVert() const { return cantVert;}
int Grafo::getMaxW() const { return maxW;}

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
    // dist[v] = costo minimo en peso de v hasta destino.
    // Como el grafo es no dirigido, la lista de adyacencia invertida coincide
    // con listaAdy, y el costo de v a destino es el mismo que de destino a v:
    // basta un Dijkstra desde el destino.
    return dijkstra(destino);
}

Nodo Grafo::getArista(int a, int b) const{
    // consulta O(1) sobre el indice ady
    auto it = ady[a].find(b);
    if (it != ady[a].end()) return it->second;
    // tiramos error ya que no se pueden devolver referencias nulas
    ostringstream msg;
    msg << "no se encontro arista desde " << a << " hasta "<< b << " error en metodo getArista";
    throw runtime_error(msg.str());
}

bool Grafo::existeArista(int origen, int final) const {
    return ady[origen].count(final) > 0;
}


int Grafo::getIdNodoInicial() const{
    return 0;
}

int Grafo::getIdNodoFinal() const{
    return (int) listaAdy.size() - 1;
}

int Grafo::getPeso(int a, int b) const{
    auto it = ady[a].find(b);
    if (it != ady[a].end()) return it->second.costo;
    throw runtime_error("no se encontro nodo en get peso");
}
int Grafo::getBeneficio(int a, int b) const{
    auto it = ady[a].find(b);
    if (it != ady[a].end()) return it->second.beneficio;
    throw runtime_error("no se encontro nodo en get beneficio");
}