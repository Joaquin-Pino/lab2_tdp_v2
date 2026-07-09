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

    void eliminarUltimo();

public:
    Camino();
    Camino(std::vector<int> camino, const Grafo& grafo);
    // operator<: compara por beneficio total. c1 < c2 significa que c1 tiene
    // MENOR beneficio (es decir, PEOR solución)
    bool operator<(const Camino& otro) const;
    bool operator>(const Camino& otro) const;

    int getPesoTotal();
    int getBeneficioTotal() const;

    int getLargo();

    bool nodoFueVisitado(int id) const;

    bool intercambiarNodos(int id1, int id2);

    //todo: implementar correctamente
    bool verificarCamino(int wMax); // verifica si camino es valido

    bool agregarNodo(int id);

    // Elimina un nodo interior y lo reemplaza por la arista puente entre sus
    // vecinos, actualizando peso y beneficio. Devuelve false (sin tocar el
    // camino) si el nodo no esta, si es un extremo, o si sus vecinos no quedan
    // unidos por una arista del grafo.
    bool eliminarNodo(int id);

    int getUltimoNodo();

    bool esCaminoCompleto();
    float getRatioNodo(int id);
    void reemplazarNodo(int oldId, int newId);
    std::vector<int> getCamino() const;

    int getPosicionNodo(int idNodo);

    bool llegaFinal();

    void concatenar(const std::vector<int> &c);

    void setPesoTotal(int p);
    void setCamino(std::vector<int> c);

    void setBeneficio(int b);
};