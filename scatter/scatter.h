#pragma once

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
#include "../grasp/grasp.h"
#include <vector>
#include <random>
#include <unordered_set>
#include <utility>

class Scatter {
public:
    // Operador de insercion de subcadena: MEJOR = best-improvement (evalua toda
    // la vecindad y aplica el injerto de mayor beneficio), PRIMERA = first-fit
    // (aplica el primer injerto factible). Se elige por parametro del ctor.
    enum ModoInsercion { MEJOR, PRIMERA };


    // umbralDensidad = 0.6: en un grafo no dirigido cada arista suma 2 al total
    // de grados, asi que la densidad duplica a la del mismo conjunto de aristas
    // leido como dirigido. El 0.6 conserva la clasificacion denso/disperso que
    // daba el 0.3 sobre la densidad dirigida.
    // rng compartido inyectado desde afuera; debe sobrevivir al Scatter. Se
    // propaga a su Grasp y al Kopt de refinar(). tamPoblacion/tamRefSet son los
    // tamanos de la busqueda dispersa: se exponen para que el Planner los pueda
    // escalar segun el grafo (defaults = valores calibrados previos).
    Scatter(const Grafo& grafo, std::mt19937& rng, int maxNodosInsertar = 3,
            double umbralDensidad = 0.6, ModoInsercion modo = MEJOR,
            int tamPoblacion = 30, int tamRefSet = 10);

    Camino resolver(int maxIter);
    Camino combinar(const Camino& C1, const Camino& C2) const;

private:
    const Grafo* grafo;
    std::mt19937* rng; // no-dueño: lo inyecta quien construye el solver
    Grasp grasp; // construccion GRASP: arma la poblacion inicial (ver generarPoblacion)

    int maxNodosInsertar;      // largo maximo de la subcadena a injertar
    double umbralDensidad;     // frontera denso/disperso sobre 2M/(N(N-1))
    bool grafoEsDenso;         // decidido una vez en el ctor
    ModoInsercion modo; // variante de insertarSubcadena a usar

    // Tamanos de la busqueda dispersa (los fija el ctor; el Planner los escala).
    int tamPoblacion;
    int tamRefSet;

    // Densidad del grafo no dirigido: delega en Grafo::getDensidad(), que es
    // 2M / (N(N-1)). Vale 1 en el grafo completo.
    double densidadGrafo() const;

    // --- Combinacion ------------------------------------------------------

    // Injerta una subcadena de 'donante' (hasta maxNodosInsertar nodos nuevos)
    // en 'base', entre un par (a,b) con aristas a->w1 y wk->b, sin repetir nodos
    // y dentro de presupuesto. mejorFactible elige best- vs first-improvement.
    // Devuelve (camino resultante, hubo injerto).
    std::pair<std::vector<int>, bool> insertarSubcadena(
        const std::vector<int>& base,
        const std::vector<int>& donante,
        bool mejorFactible) const;

    // Empalme (operador para grafos dispersos): primera mitad de C1 + segunda
    // mitad de C2 (o al reves). Solo necesita 1 arista en la union.
    Camino empalme(const Camino& C1, const Camino& C2) const;

    // Une la primera mitad de 'primera' con la segunda mitad de 'segunda' si el
    // empalme existe como arista, no repite nodos y respeta el peso. (camino, ok).
    std::pair<std::vector<int>, bool> intentarUnion(
        const std::vector<int>& primera,
        const std::vector<int>& segunda) const;

    bool sinDuplicados(const std::vector<int>& mitad1,
                       const std::vector<int>& mitad2) const;

    // Refina una solucion con 2-opt (reordena para liberar peso). El agregado
    // de nodos ya lo hace la combinacion.
    Camino refinar(const Camino& solucion) const;

    // --- RefSet -----------------------------------------------------------

    // Deduplica por camino y devuelve los b de mayor beneficio.
    std::vector<Camino> seleccionarRefSet(std::vector<Camino> candidatos, int b) const;

    // True si dos refSet (ya ordenados por seleccionarRefSet) son iguales.
    bool mismosRefSet(const std::vector<Camino>& a,
                      const std::vector<Camino>& b) const;


};
