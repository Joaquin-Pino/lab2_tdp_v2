#pragma once
#include <vector>
#include <unordered_set>
#include "../grafo/grafo.h"

class Camino {
private:
    int pesoTotal;
    int beneficioTotal;
    std::unordered_set<int> visitados; // guarda id de nodos visitados, tabla hash por lo que consulta es o(1)
    std::vector<int> camino;

    const Grafo* grafo;
    int calcularPesoTotal(); // implementar si es necesario
    int calcularBeneficioTotal(); // implementar si es necesario
    void calcularYAsignarPesoYBeneficio();

public:
    Camino();
    Camino(std::vector<int> camino, const Grafo& grafo);

    int getPesoTotal();
    int getBeneficioTotal();

    int getLargo();

    bool nodoFueVisitado(int id) const;

    bool intercambiarNodos(int id1, int id2);

    //todo: implementar correctamente
    bool verificarCamino(int wMax); // verifica si camino es valido

    void agregarNodo(int id);
    void eliminarNodo(int id);

    int getUltimoNodo();

    bool esCaminoCompleto();
    float getRatioNodo(int id);
    void reemplazarNodo(int oldId, int newId);
    std::vector<int> getCamino() const;

    int getPosicionNodo(int idNodo);
};