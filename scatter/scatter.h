#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
#include <vector>
#include <random>

class Scatter {
private:
    const Grafo* grafo;

    std::vector<Camino> generarSoluciones(int n);

    std::mt19937 rng;

    // Verifica que dos secuencias de nodos no comparten ningún nodo entre sí
    bool sinDuplicados(const std::vector<int>& mitad1,
                        const std::vector<int>& mitad2) const;

    // Intenta construir un camino uniendo la primera mitad de 'primera' con
    // la segunda mitad de 'segunda'. Retorna (camino, es_valido).
    std::pair<std::vector<int>, bool> intentarUnion(
        const std::vector<int>& primera,
        const std::vector<int>& segunda) const;

    Camino construccionAleatoria();

    struct CandidatoInsercion {
        int nodo;
        size_t posicion;       // se inserta entre camino[posicion] y camino[posicion+1]
        double deltaBeneficio;
        double deltaPeso;
        double eficiencia;
    };
    
    std::vector<CandidatoInsercion> generarCandidatos(const std::vector<int>& camino, double pesoActual) const;

    std::vector<Camino> generarSolucionesAleatorias(int n);

    // Hash de un vector<int>, usado para identificar caminos en refSet/usados
    struct VectorIntHash {
        size_t operator()(const std::vector<int>& v) const;
    };

    // Hash de un par de caminos (por su secuencia de nodos)
    struct ParHash {
        size_t operator()(const std::pair<std::vector<int>, std::vector<int>>& par) const;
    };

    // Clave de un par de caminos, sin importar el orden en que se pasen
    static std::pair<std::vector<int>, std::vector<int>> clavePar(const Camino& a, const Camino& b);

public:

    Scatter();
    Scatter(const Grafo& grafo);

    Camino resolver(int maxIter);
    Camino combinar(const Camino& C1, const Camino& C2) const;
};