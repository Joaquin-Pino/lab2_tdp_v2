#include <cassert>
#include <iostream>
#include "kopt.h"

// Grafo con una arista alternativa 0->2->1->3 que mejora el camino
// original 0->1->2->3->4. maxW=10, holgado para que solo importe el
// beneficio.
// Camino original 0-1-2-3-4: peso=4, beneficio=4
// Camino tras intercambiar posiciones 1 y 2 (0-2-1-3-4): peso=4, beneficio=7
Grafo crearGrafoConMejoraSwap() {
    Grafo g(5, 7, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    g.insertarArista(0, 2, 1, 2);
    g.insertarArista(2, 1, 1, 2);
    g.insertarArista(1, 3, 1, 2);
    return g;
}

void test_resolverInicialMejoraConSwapK2() {
    Grafo g = crearGrafoConMejoraSwap();
    Kopt kopt(g, 2);
    Camino inicial(std::vector<int>{0, 1, 2, 3, 4}, g);

    Camino resultado = kopt.resolver(inicial);

    assert((resultado.getCamino() == std::vector<int>{0, 2, 1, 3, 4}));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 7);
    std::cout << "test_resolverInicialMejoraConSwapK2: OK\n";
}

// Solo existe el camino lineal 0->1->2->3->4: ninguna combinacion/permutacion
// de k=2 nodos tiene todas sus aristas en el grafo, asi que verificarAristas
// las descarta todas y el camino no debe cambiar.
Grafo crearGrafoSinAristasAlternativas() {
    Grafo g(5, 4, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    return g;
}

void test_resolverInicialSinAristasValidasNoCambia() {
    Grafo g = crearGrafoSinAristasAlternativas();
    Kopt kopt(g, 2);
    Camino inicial(std::vector<int>{0, 1, 2, 3, 4}, g);

    Camino resultado = kopt.resolver(inicial);

    assert((resultado.getCamino() == std::vector<int>{0, 1, 2, 3, 4}));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 4);
    std::cout << "test_resolverInicialSinAristasValidasNoCambia: OK\n";
}

// Igual que crearGrafoConMejoraSwap, pero el intercambio de 1 y 2 pasa a
// costar mas de lo que queda de presupuesto (maxW=4): las aristas existen
// y el beneficio del swap (12) es mayor, pero su peso (6) excede maxW, asi
// que debe descartarse y el camino original debe quedar intacto.
Grafo crearGrafoQueExcedePresupuesto() {
    Grafo g(5, 7, 4);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    g.insertarArista(0, 2, 2, 5);
    g.insertarArista(2, 1, 2, 5);
    g.insertarArista(1, 3, 1, 1);
    return g;
}

void test_resolverInicialRespetaPresupuesto() {
    Grafo g = crearGrafoQueExcedePresupuesto();
    Kopt kopt(g, 2);
    Camino inicial(std::vector<int>{0, 1, 2, 3, 4}, g);

    Camino resultado = kopt.resolver(inicial);

    assert((resultado.getCamino() == std::vector<int>{0, 1, 2, 3, 4}));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 4);
    std::cout << "test_resolverInicialRespetaPresupuesto: OK\n";
}

// Las razones beneficio/costo estan armadas para que SolverGreedy elija
// 0->1->2->3->4 (peso=4, beneficio=7) y luego el K-OPT encuentre el swap
// de las posiciones 1 y 2 hacia 0->2->1->3->4 (peso=5, beneficio=8).
// Verifica que resolver() sin argumentos delega en el goloso antes de
// refinar.
Grafo crearGrafoParaResolverSinArgumentos() {
    Grafo g(5, 7, 10);
    g.insertarArista(0, 1, 1, 3);
    g.insertarArista(0, 2, 1, 1);
    g.insertarArista(1, 2, 1, 2);
    g.insertarArista(1, 3, 2, 3);
    g.insertarArista(2, 1, 1, 3);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    return g;
}

void test_resolverSinArgumentosDelegaEnGreedyYMejora() {
    Grafo g = crearGrafoParaResolverSinArgumentos();
    Kopt kopt(g, 2);

    Camino resultado = kopt.resolver();

    assert((resultado.getCamino() == std::vector<int>{0, 2, 1, 3, 4}));
    assert(resultado.getPesoTotal() == 5);
    assert(resultado.getBeneficioTotal() == 8);
    std::cout << "test_resolverSinArgumentosDelegaEnGreedyYMejora: OK\n";
}

// k=3: el camino original 0-1-2-3-4-5 (peso=5, beneficio=5) mejora
// reordenando las posiciones 1,2,3 hacia 0-3-1-2-4-5 (peso=5, beneficio=8),
// usando las aristas alternativas 0->3, 3->1 y 2->4.
Grafo crearGrafoConMejoraK3() {
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

void test_resolverInicialMejoraConK3() {
    Grafo g = crearGrafoConMejoraK3();
    Kopt kopt(g, 3);
    Camino inicial(std::vector<int>{0, 1, 2, 3, 4, 5}, g);

    Camino resultado = kopt.resolver(inicial);

    assert((resultado.getCamino() == std::vector<int>{0, 3, 1, 2, 4, 5}));
    assert(resultado.getPesoTotal() == 5);
    assert(resultado.getBeneficioTotal() == 8);
    std::cout << "test_resolverInicialMejoraConK3: OK\n";
}

int main() {
    test_resolverInicialMejoraConSwapK2();
    test_resolverInicialSinAristasValidasNoCambia();
    test_resolverInicialRespetaPresupuesto();
    test_resolverSinArgumentosDelegaEnGreedyYMejora();
    test_resolverInicialMejoraConK3();
    std::cout << "--- Todos los tests de Kopt pasaron ---\n";
    return 0;
}
