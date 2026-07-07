#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
#include <vector>
#include <random>
#include <unordered_set>

class Scatter {
private:
    const Grafo* grafo;
    std::mt19937 rng;

    // ===== Poblacion inicial =====

    // n perturbaciones del ancla + el ancla golosa.
    std::vector<Camino> generarSoluciones(int n);
    std::vector<Camino> generarSolucionesAleatorias(int n);

    // Construccion por perturbacion (ruin-and-recreate): conserva un prefijo
    // aleatorio del ancla golosa y reconstruye la cola al azar (RCL por
    // eficiencia) hasta el destino. El prefijo aporta calidad; la cola, la
    // diversidad que la combinacion necesita. distInv y anclaCamino se pasan ya
    // calculados (son iguales en toda la poblacion).
    Camino construccionAleatoria(const std::vector<int>& anclaCamino,
                                 const std::vector<int>& distInv);

    // Solucion del goloso completada hasta el destino: ancla de calidad de la
    // poblacion (el goloso es el unico constructor que crece caminos largos en
    // grafos dispersos).
    Camino solucionGreedy() const;

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

    // ===== Combinacion =====

    // Une la primera mitad de 'primera' con la segunda mitad de 'segunda' si el
    // empalme existe como arista, no repite nodos y respeta el peso. (camino, ok).
    std::pair<std::vector<int>, bool> intentarUnion(
        const std::vector<int>& primera,
        const std::vector<int>& segunda) const;

    bool sinDuplicados(const std::vector<int>& mitad1,
                       const std::vector<int>& mitad2) const;

    // ===== Mejora local =====

    // Refina una solucion: 2-OPT (reordena) + rellenarPresupuesto de 1 nodo
    // (agrega beneficio en el peso liberado). Es el paso de mejora estandar.
    Camino refinar(const Camino& solucion) const;

    // Rellena el presupuesto ocioso reemplazando aristas (a,b) por un sub-camino
    // a->w1->...->wk->b de hasta 'maxNodos' nodos nuevos (detour). A diferencia
    // del 2-OPT (que solo reordena), AGREGA beneficio: es lo que permite superar
    // el optimo local del 2-OPT. maxNodos=1 => inserciones de un solo nodo.
    Camino rellenarPresupuesto(const Camino& solucion, int maxNodos) const;

    // Un detour candidato que reemplaza una arista (a,b).
    struct Detour {
        std::vector<int> nodos;  // nodos intermedios nuevos w1..wk (k >= 1)
        int deltaPeso = 0;       // cambio de peso del camino al aplicarlo
        int deltaBenef = 0;      // cambio de beneficio (solo se aceptan > 0)
    };

    // DFS acotado a 'maxNodos' nodos que actualiza 'mejor' con el detour de
    // mayor ganancia de beneficio desde 'actual' hasta 'destino'.
    void buscarDetour(int actual, int destino, int maxNodos,
                      int pesoDisponible, int pesoBase, int benefBase,
                      int pesoAcum, int benefAcum,
                      const std::unordered_set<int>& enCamino,
                      std::vector<int>& actualNodos,
                      std::unordered_set<int>& usadosLocal,
                      Detour& mejor) const;

    // ===== RefSet: claves y hashing de pares ya combinados =====

    struct VectorIntHash {
        size_t operator()(const std::vector<int>& v) const;
    };
    struct ParHash {
        size_t operator()(const std::pair<std::vector<int>, std::vector<int>>& par) const;
    };
    // Clave de un par de caminos, independiente del orden en que se pasen.
    static std::pair<std::vector<int>, std::vector<int>> clavePar(const Camino& a,
                                                                  const Camino& b);

public:
    Scatter();
    Scatter(const Grafo& grafo);

    Camino resolver(int maxIter);
    Camino combinar(const Camino& C1, const Camino& C2) const;
};
