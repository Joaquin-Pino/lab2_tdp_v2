#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
#include <vector>
#include <random>
#include <unordered_set>

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

    struct CandidatoExtension {
        int nodo;
        int costo;
        int beneficio;
        double eficiencia;     // beneficio/costo de la arista actual->nodo
    };

    // Candidatos para EXTENDER el camino desde 'actual': vecinos no visitados
    // que todavia dejan una completacion factible en peso hasta el destino
    // (segun distInv = dijkstraInvertido).
    std::vector<CandidatoExtension> candidatosExtension(
        int actual, double pesoActual,
        const std::unordered_set<int>& enCamino,
        const std::vector<int>& distInv) const;

    // Cierra un camino parcial hasta el destino con el camino mas corto,
    // evitando reutilizar nodos ya visitados (retrocede el tramo extendido si
    // hiciera falta). Recibe el camino y su conjunto de visitados por copia.
    std::vector<int> completarHastaDestino(std::vector<int> camino,
                                           std::unordered_set<int> enCamino) const;

    // Solucion del goloso completada hasta el destino: ancla de calidad que
    // se inyecta en la poblacion inicial (el goloso es el unico constructor
    // que crece caminos largos en grafos dispersos).
    Camino solucionGreedy() const;

    // Rellena el presupuesto ocioso insertando nodos no visitados entre pares
    // consecutivos (a,b) cuando existen las aristas a->u y u->b. A diferencia
    // del 2-OPT (que solo reordena), este operador AGREGA beneficio, que es lo
    // unico que permite a Scatter superar el optimo local del 2-OPT. Aplica
    // inserciones de mayor ganancia de beneficio hasta que ninguna sea factible.
    Camino repararEInsertar(const Camino& solucion) const;

    // Un detour: sub-camino de nodos nuevos que reemplaza una arista (a,b).
    struct Detour {
        std::vector<int> nodos;  // nodos intermedios nuevos w1..wk (k >= 2)
        int deltaPeso = 0;       // cambio de peso del camino al aplicarlo
        int deltaBenef = 0;      // cambio de beneficio (solo se aceptan > 0)
    };

    // Generaliza repararEInsertar a SEGMENTOS: reemplaza una arista (a,b) por
    // un sub-camino a->w1->...->wk->b (k >= 2 nodos nuevos), aun cuando no
    // exista el triangulo directo a->u->b. Explora detours por DFS acotado a
    // 'maxNodos' nodos intermedios y aplica, por best-improvement, el de mayor
    // ganancia de beneficio que quepa en el presupuesto, hasta converger.
    Camino insertarSegmentos(const Camino& solucion, int maxNodos) const;

    // DFS recursivo que busca el mejor detour desde 'actual' hasta 'destino'
    // (extremos de la arista reemplazada). Actualiza 'mejor' con el detour de
    // mayor deltaBenef encontrado. Metodo (no funcion libre) por el rubro.
    void buscarDetour(int actual, int destino, int maxNodos,
                      int pesoDisponible, int pesoBase, int benefBase,
                      int pesoAcum, int benefAcum,
                      const std::unordered_set<int>& enCamino,
                      std::vector<int>& actualNodos,
                      std::unordered_set<int>& usadosLocal,
                      Detour& mejor) const;

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