#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "../nodo/nodo.h"

class Grafo {
private:
    int cantVert;
    int cantArist;
    int maxW;
    std::vector<std::vector<Nodo>> listaAdy;
    // Indice paralelo origen -> (destino -> Nodo) para consultas de arista O(1)
    // (getPeso/getBeneficio/existeArista/getArista). Se llena junto con listaAdy.
    std::vector<std::unordered_map<int, Nodo>> ady;

public:
    Grafo(int cantVert, int cantArist, int maxW);

    static Grafo cargarDesdeArchivo(const std::string& filename);

    void insertarArista(int origen, int destino, int costo, int beneficio);

    const std::vector<Nodo>& getVecinos(int idNodo) const;
    int getCantVert() const;
    int getMaxW() const;
    int getNodoDestino() const;

    std::vector<int> dijkstra(int origen) const;
    std::vector<int> dijkstraInvertido(int destino) const;

    std::vector<int> dijkstraCamino(int origen, int destino) const;
    float getRatioMejorEntrada(int id) const;

    Nodo getArista(int a, int b)const;

    bool existeArista(int origen, int destino) const;

    int getIdNodoInicial() const;
    int getIdNodoFinal() const;

    int getPeso(int a, int b) const;
    int getBeneficio(int a, int b) const;
};
