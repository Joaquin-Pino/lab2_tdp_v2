#pragma once

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
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

private:
    const Grafo* grafo;
    std::mt19937 rng;

    int maxNodosInsertar;      // largo maximo de la subcadena a injertar
    double umbralDensidad;     // frontera denso/disperso sobre 2M/(N(N-1))
    bool grafoEsDenso;         // decidido una vez en el ctor
    ModoInsercion modo;        // variante de insertarSubcadena a usar

    // Tamanos de la busqueda dispersa (calibrables).
    static constexpr int TAM_POBLACION = 30;
    static constexpr int TAM_REFSET = 10;

    // Densidad del grafo no dirigido: suma de grados / pares ordenados posibles,
    // o sea 2M / (N(N-1)), con M = cantidad de aristas no dirigidas. Vale 1 en
    // el grafo completo.
    double densidadGrafo() const;

    // --- Construccion GRASP ----------------------------------------------

    struct CandidatoExtension {
        int nodo;
        int costo;
        int beneficio;
        double eficiencia;     // beneficio/costo de la arista actual->nodo
    };

    // Vecinos no visitados de 'actual' que aun dejan una completacion factible
    // en peso hasta el destino (segun distInv = dijkstraInvertido).
    std::vector<CandidatoExtension> candidatosExtension(
        int actual, double pesoActual,
        const std::unordered_set<int>& enCamino,
        const std::vector<int>& distInv) const;

    // Cierra un camino parcial hasta el destino con el camino mas corto, sin
    // reutilizar nodos (retrocede el tramo extendido si hiciera falta).
    std::vector<int> completarHastaDestino(std::vector<int> camino,
                                           std::unordered_set<int> enCamino) const;

    // Una construccion golosa aleatorizada (GRASP): parte de [0] y extiende por
    // una lista restringida de candidatos (RCL) segun eficiencia, cerrando al
    // destino. distInv evita meterse en callejones sin salida en peso.
    Camino construirGrasp(const std::vector<int>& distInv);

    // n construcciones GRASP, cada una refinada con 2-opt.
    std::vector<Camino> generarPoblacion(int n);

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

public:
    Scatter();
    // umbralDensidad = 0.6: en un grafo no dirigido cada arista suma 2 al total
    // de grados, asi que la densidad duplica a la del mismo conjunto de aristas
    // leido como dirigido. El 0.6 conserva la clasificacion denso/disperso que
    // daba el 0.3 sobre la densidad dirigida.
    Scatter(const Grafo& grafo, int maxNodosInsertar = 3,
            double umbralDensidad = 0.6, ModoInsercion modo = MEJOR);

    Camino resolver(int maxIter);
    Camino combinar(const Camino& C1, const Camino& C2) const;
};
