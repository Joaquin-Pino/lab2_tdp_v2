#pragma once
#include <vector>
#include <string>
#include "../nodo/nodo.h"

class Grafo {
private:
    int cantVert;
    int cantArist;
    int maxW;
    std::vector<std::vector<Nodo>> listaAdy;
    
public:
    Grafo(int cantVert, int cantArist, int maxW);

    static Grafo cargarDesdeArchivo(const std::string& filename);

    void insertarArista(int origen, int destino, int costo, int beneficio);

    std::vector<Nodo> getVecinos(int idNodo) const;
    int getCantVert() const;
    int getMaxW() const;
    int getNodoDestino() const;

    std::vector<int> dijkstra(int origen) const;
    std::vector<int> dijkstraInvertido(int destino) const;

    std::vector<int> dijkstraCamino(int origen, int destino) const;
    float getRatioMejorEntrada(int id) const;

    Nodo getArista(int a, int b)const;

    bool existeArista(int origen, int destino) const;
};
