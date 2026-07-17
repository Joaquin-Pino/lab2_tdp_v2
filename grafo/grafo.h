#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "../nodo/nodo.h"

// Grafo NO DIRIGIDO con pesos (costo) y beneficios en las aristas, guardado
// como lista de adyacencia. Cada arista {u,v} aparece en listaAdy[u] y en
// listaAdy[v] con el mismo costo y beneficio, por lo que existeArista(u,v)
// == existeArista(v,u) y getArista(u,v) == getArista(v,u).
class Grafo {
private:
    int cantVert;
    int maxW;
    std::vector<std::vector<Nodo>> listaAdy;
    // Indice paralelo extremo -> (vecino -> Nodo) para consultas de arista O(1)
    // (getPeso/getBeneficio/existeArista/getArista). Se llena junto con listaAdy
    // y guarda las mismas aristas: nunca hay una en uno y no en el otro.
    std::vector<std::unordered_map<int, Nodo>> ady;

public:
    Grafo(int cantVert, int cantArist, int maxW);

    static Grafo cargarDesdeArchivo(const std::string& filename);

    // Inserta la arista no dirigida {origen, destino} en ambos extremos.
    // Ignora los lazos (origen == destino) y las aristas ya declaradas en
    // cualquiera de los dos sentidos, conservando la primera lectura.
    void insertarArista(int origen, int destino, int costo, int beneficio);

    const std::vector<Nodo>& getVecinos(int idNodo) const;
    int getCantVert() const;
    int getMaxW() const;

    // Cantidad de aristas no dirigidas: se cuenta desde listaAdy (suma de grados
    // / 2) porque el ctor descarta el cantArist del archivo. Recorre la lista de
    // adyacencia en O(N + M).
    int getCantAristas() const;

    // Densidad del grafo no dirigido: 2M / (N(N-1)), o sea suma de grados sobre
    // pares ordenados posibles. Vale 1 en el grafo completo y 0 si N <= 1. Es la
    // misma metrica que usa el Planner para elegir la heuristica de cota y que
    // Scatter usa para elegir su operador de combinacion.
    double getDensidad() const;

    std::vector<int> dijkstra(int origen) const;

    std::vector<int> dijkstraCamino(int origen, int destino) const;

    Nodo getArista(int a, int b)const;

    bool existeArista(int origen, int destino) const;

    int getIdNodoInicial() const;
    int getIdNodoFinal() const;

    int getPeso(int a, int b) const;
    int getBeneficio(int a, int b) const;
};
