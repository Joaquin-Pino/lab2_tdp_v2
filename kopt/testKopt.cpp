#include <cassert>
#include <iostream>
#include <random>
#include "kopt.h"

class TestKopt {
public:

    // Grafo no dirigido donde el desvio 0-2-1-3-4 mejora al camino original
    // 0-1-2-3-4. maxW=10, holgado para que solo importe el beneficio.
    // La arista {1,2} es la misma en ambos caminos; lo que cambia es entrar por
    // {0,2} (beneficio 2) y salir por {1,3} (beneficio 2) en vez de usar {0,1} y
    // {2,3} (beneficio 1 cada una).
    // Camino original 0-1-2-3-4: peso=4, beneficio=1+1+1+1=4
    // Camino tras intercambiar posiciones 1 y 2 (0-2-1-3-4): peso=4, beneficio=2+1+2+1=6
    static Grafo crearGrafoConMejoraSwap() {
        Grafo g(5, 6, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(1, 2, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(3, 4, 1, 1);
        g.insertarArista(0, 2, 1, 2);
        g.insertarArista(1, 3, 1, 2);
        return g;
    }

    static void test_resolverInicialMejoraConSwapK2() {
        Grafo g = crearGrafoConMejoraSwap();
        std::mt19937 rng(2);
        Kopt kopt(g, rng);
        Camino inicial(std::vector<int>{0, 1, 2, 3, 4}, g);

        Camino resultado = kopt.resolver(inicial);

        assert((resultado.getCamino() == std::vector<int>{0, 2, 1, 3, 4}));
        assert(resultado.getPesoTotal() == 4);
        assert(resultado.getBeneficioTotal() == 6);
        std::cout << "test_resolverInicialMejoraConSwapK2: OK\n";
    }

    // Solo existe el camino lineal 0->1->2->3->4: ninguna combinacion/permutacion
    // de k=2 nodos tiene todas sus aristas en el grafo, asi que verificarAristas
    // las descarta todas y el camino no debe cambiar.
    static Grafo crearGrafoSinAristasAlternativas() {
        Grafo g(5, 4, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(1, 2, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(3, 4, 1, 1);
        return g;
    }

    static void test_resolverInicialSinAristasValidasNoCambia() {
        Grafo g = crearGrafoSinAristasAlternativas();
        std::mt19937 rng(2);
        Kopt kopt(g, rng);
        Camino inicial(std::vector<int>{0, 1, 2, 3, 4}, g);

        Camino resultado = kopt.resolver(inicial);

        assert((resultado.getCamino() == std::vector<int>{0, 1, 2, 3, 4}));
        assert(resultado.getPesoTotal() == 4);
        assert(resultado.getBeneficioTotal() == 4);
        std::cout << "test_resolverInicialSinAristasValidasNoCambia: OK\n";
    }

    // Igual que crearGrafoConMejoraSwap, pero el intercambio de 1 y 2 pasa a
    // costar mas de lo que queda de presupuesto (maxW=4): las aristas existen
    // y el beneficio del swap (0-2-1-3-4 da 5+1+1+1=8) es mayor, pero su peso
    // (2+1+1+1=5) excede maxW, asi que debe descartarse y el camino original
    // debe quedar intacto.
    static Grafo crearGrafoQueExcedePresupuesto() {
        Grafo g(5, 6, 4);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(1, 2, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(3, 4, 1, 1);
        g.insertarArista(0, 2, 2, 5);
        g.insertarArista(1, 3, 1, 1);
        return g;
    }

    static void test_resolverInicialRespetaPresupuesto() {
        Grafo g = crearGrafoQueExcedePresupuesto();
        std::mt19937 rng(2);
        Kopt kopt(g, rng);
        Camino inicial(std::vector<int>{0, 1, 2, 3, 4}, g);

        Camino resultado = kopt.resolver(inicial);

        assert((resultado.getCamino() == std::vector<int>{0, 1, 2, 3, 4}));
        assert(resultado.getPesoTotal() == 4);
        assert(resultado.getBeneficioTotal() == 4);
        std::cout << "test_resolverInicialRespetaPresupuesto: OK\n";
    }

    // Las razones beneficio/costo estan armadas para que SolverGreedy elija
    // 0-1-2-3-4 (peso=4, beneficio=3+2+1+1=7): desde 0 prefiere {0,1} (razon 3)
    // sobre {0,2} (razon 2), y desde 1 prefiere {1,2} (razon 2) sobre {1,3}
    // (razon 1). Luego el K-OPT encuentra el swap de las posiciones 1 y 2 hacia
    // 0-2-1-3-4 (peso=2+1+1+1=5, beneficio=4+2+1+1=8), que el goloso no ve porque
    // entrar por la arista cara {0,2} solo conviene mirando el camino completo.
    // Verifica que resolver() sin argumentos delega en el goloso antes de refinar.
    static Grafo crearGrafoParaResolverSinArgumentos() {
        Grafo g(5, 6, 10);
        g.insertarArista(0, 1, 1, 3);
        g.insertarArista(0, 2, 2, 4);
        g.insertarArista(1, 2, 1, 2);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(3, 4, 1, 1);
        return g;
    }

    static void test_resolverSinArgumentosDelegaEnGreedyYMejora() {
        Grafo g = crearGrafoParaResolverSinArgumentos();
        std::mt19937 rng(2);
        Kopt kopt(g, rng);

        Camino resultado = kopt.resolver();

        assert((resultado.getCamino() == std::vector<int>{0, 2, 1, 3, 4}));
        assert(resultado.getPesoTotal() == 5);
        assert(resultado.getBeneficioTotal() == 8);
        std::cout << "test_resolverSinArgumentosDelegaEnGreedyYMejora: OK\n";
    }

    // k=3: el camino original 0-1-2-3-4-5 (peso=5, beneficio=5) mejora
    // reordenando las posiciones 1,2,3 hacia 0-3-1-2-4-5 (peso=5, beneficio=8),
    // usando las aristas alternativas 0->3, 3->1 y 2->4.
    static Grafo crearGrafoConMejoraK3() {
        Grafo g(6, 8, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(1, 2, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(3, 4, 1, 1);
        g.insertarArista(4, 5, 1, 1);
        g.insertarArista(0, 3, 1, 2);
        g.insertarArista(3, 1, 1, 2);
        g.insertarArista(2, 4, 1, 2);
        return g;
    }

    static void test_resolverInicialMejoraConK3() {
        Grafo g = crearGrafoConMejoraK3();
        std::mt19937 rng(2);
        Kopt kopt(g, rng);
        Camino inicial(std::vector<int>{0, 1, 2, 3, 4, 5}, g);

        // k=3 explicito: el constructor no fija un k por defecto (el rng que
        // recibe el ctor solo lo usan granSalto/perturbar, no resolver()).
        Camino resultado = kopt.resolver(inicial, true, 3);

        assert((resultado.getCamino() == std::vector<int>{0, 3, 1, 2, 4, 5}));
        assert(resultado.getPesoTotal() == 5);
        assert(resultado.getBeneficioTotal() == 8);
        std::cout << "test_resolverInicialMejoraConK3: OK\n";
    }
};

int main() {
    TestKopt::test_resolverInicialMejoraConSwapK2();
    TestKopt::test_resolverInicialSinAristasValidasNoCambia();
    TestKopt::test_resolverInicialRespetaPresupuesto();
    TestKopt::test_resolverSinArgumentosDelegaEnGreedyYMejora();
    TestKopt::test_resolverInicialMejoraConK3();
    std::cout << "--- Todos los tests de Kopt pasaron ---\n";
    return 0;
}
