#pragma once

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
#include "../grasp/grasp.h"
#include <vector>
#include <random>
#include <unordered_set>
#include <utility>
#include <climits>

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
    // propaga a su Grasp y al Kopt de refinar().
    Scatter(const Grafo& grafo, std::mt19937& rng, int maxNodosInsertar = 3,
            double umbralDensidad = 0.6, ModoInsercion modo = MEJOR);

    // maxCombinaciones: tope determinista de combinaciones (combinar+refinar)
    // en total. Al alcanzarlo corta y devuelve el mejor camino visto. Acota el
    // trabajo en grafos grandes (donde el 2-opt de refinar escala con el largo
    // del camino) sin depender del reloj, asi el resultado es reproducible.
    Camino resolver(int maxIter, long maxCombinaciones = LONG_MAX);
    Camino combinar(const Camino& C1, const Camino& C2) const;

private:
    const Grafo* grafo;
    std::mt19937* rng; // no-dueño: lo inyecta quien construye el solver
    Grasp grasp; // construccion GRASP: arma la poblacion inicial (ver generarPoblacion)

    int maxNodosInsertar;      // largo maximo de la subcadena a injertar
    double umbralDensidad;     // frontera denso/disperso sobre 2M/(N(N-1))
    bool grafoEsDenso;         // decidido una vez en el ctor
    ModoInsercion modo; // variante de insertarSubcadena a usar

    // Tamanos de la busqueda dispersa (calibrables).
    static constexpr int TAM_POBLACION = 30;
    static constexpr int TAM_REFSET = 10;
    // Cuantos combos crudos (los de mayor beneficio) se refinan por ronda. El
    // refinar() con 2-opt es lo caro, asi que en vez de refinar los ~45 combos
    // se refina solo este pool de prometedores y despues se reselecciona el
    // refSet. 2*TAM_REFSET deja margen para que un combo que mejora mucho al
    // refinarse no se pierda por su beneficio crudo.
    static constexpr int POOL_REFINAR = 2 * TAM_REFSET;
    // Sobre este largo de camino, refinar() acota el 2-opt a MAX_PASADAS_REFINE
    // pasadas (cada pasada es O(L^2)); en caminos mas cortos corre a convergencia.
    static constexpr int UMBRAL_LARGO_REFINE = 200;
    static constexpr int MAX_PASADAS_REFINE = 2;

    // Densidad del grafo no dirigido: suma de grados / pares ordenados posibles,
    // o sea 2M / (N(N-1)), con M = cantidad de aristas no dirigidas. Vale 1 en
    // el grafo completo.
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
