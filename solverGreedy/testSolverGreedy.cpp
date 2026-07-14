#include <cassert>
#include <iostream>
#include "solverGreedy.h"

class TestSolverGreedy {
public:

    // Grafo lineal sin alternativas: 0->1->2->3 (igual al ejemplo del enunciado)
    static Grafo crearGrafoLineal() {
        Grafo g(4, 4, 7);
        g.insertarArista(0, 1, 1, 4);
        g.insertarArista(1, 2, 2, 3);
        g.insertarArista(2, 3, 3, 2);
        g.insertarArista(3, 0, 4, 1);
        return g;
    }

    static void test_resolverCaminoLineal() {
        Grafo g = crearGrafoLineal();
        SolverGreedy solver(g);

        Camino sol = solver.resolver();

        assert((sol.getCamino() == std::vector<int>{0, 1, 2, 3}));
        assert(sol.getPesoTotal() == 6);
        assert(sol.getBeneficioTotal() == 9);
        assert(sol.esCaminoCompleto());
        std::cout << "test_resolverCaminoLineal: OK\n";
    }

    // 0 tiene dos vecinos: 1 (beneficio=10, costo=5, razon=2) y 2 (beneficio=8, costo=1, razon=8).
    // obtenerMejorNodo debe elegir por mayor razon beneficio/costo, por lo que debe
    // preferir 2 aunque 1 tenga mayor beneficio absoluto.
    static Grafo crearGrafoEleccion() {
        Grafo g(4, 4, 20);
        g.insertarArista(0, 1, 5, 10);
        g.insertarArista(0, 2, 1, 8);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        return g;
    }

    static void test_resolverEligeMejorRazonBeneficioCosto() {
        Grafo g = crearGrafoEleccion();
        SolverGreedy solver(g);

        Camino sol = solver.resolver();

        assert((sol.getCamino() == std::vector<int>{0, 2, 3}));
        assert(sol.getPesoTotal() == 2);
        assert(sol.getBeneficioTotal() == 9);
        std::cout << "test_resolverEligeMejorRazonBeneficioCosto: OK\n";
    }

    // 0 tiene dos vecinos con razones fraccionarias: 1 (beneficio=9, costo=10, razon=0.9)
    // y 2 (beneficio=1, costo=2, razon=0.5). La razon correcta favorece a 1, pero si la
    // comparacion trunca a entero (9/10=0, 1/2=0) o compara contra el beneficio crudo
    // del candidato anterior, el resultado se invierte. Este test exige razon en punto
    // flotante real para pasar.
    static Grafo crearGrafoRazonFraccionaria() {
        Grafo g(4, 4, 20);
        g.insertarArista(0, 1, 10, 9);
        g.insertarArista(0, 2, 2, 1);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        return g;
    }

    static void test_resolverEligeMejorRazonFraccionaria() {
        Grafo g = crearGrafoRazonFraccionaria();
        SolverGreedy solver(g);

        Camino sol = solver.resolver();

        assert((sol.getCamino() == std::vector<int>{0, 1, 3}));
        assert(sol.getPesoTotal() == 11);
        assert(sol.getBeneficioTotal() == 10);
        std::cout << "test_resolverEligeMejorRazonFraccionaria: OK\n";
    }

    // 0 tiene un vecino caro e inalcanzable (costo=8 > maxW=5) y uno barato.
    // El solver debe descartar el caro por exceder el peso disponible.
    static Grafo crearGrafoPesoLimitado() {
        Grafo g(4, 4, 5);
        g.insertarArista(0, 1, 8, 20);
        g.insertarArista(0, 2, 2, 5);
        g.insertarArista(2, 3, 1, 3);
        return g;
    }

    static void test_resolverRespetaPesoDisponible() {
        Grafo g = crearGrafoPesoLimitado();
        SolverGreedy solver(g);

        Camino sol = solver.resolver();

        assert((sol.getCamino() == std::vector<int>{0, 2, 3}));
        assert(sol.getPesoTotal() == 3);
        assert(sol.getBeneficioTotal() == 8);
        std::cout << "test_resolverRespetaPesoDisponible: OK\n";
    }

    // El nodo 1 no tiene salida y no es el destino (2): el solver debe detenerse
    // ahi y entregar un camino incompleto en lugar de fallar.
    static Grafo crearGrafoSinSalida() {
        Grafo g(3, 1, 10);
        g.insertarArista(0, 1, 1, 5);
        return g;
    }

    static void test_resolverCaminoSinSalidaQuedaIncompleto() {
        Grafo g = crearGrafoSinSalida();
        SolverGreedy solver(g);

        Camino sol = solver.resolver();

        assert((sol.getCamino() == std::vector<int>{0, 1}));
        assert(sol.getPesoTotal() == 1);
        assert(sol.getBeneficioTotal() == 5);
        assert(!sol.esCaminoCompleto());
        std::cout << "test_resolverCaminoSinSalidaQuedaIncompleto: OK\n";
    }
};

int main() {
    TestSolverGreedy::test_resolverCaminoLineal();
    TestSolverGreedy::test_resolverEligeMejorRazonBeneficioCosto();
    TestSolverGreedy::test_resolverEligeMejorRazonFraccionaria();
    TestSolverGreedy::test_resolverRespetaPesoDisponible();
    TestSolverGreedy::test_resolverCaminoSinSalidaQuedaIncompleto();
    std::cout << "--- Todos los tests de SolverGreedy pasaron ---\n";
    return 0;
}
