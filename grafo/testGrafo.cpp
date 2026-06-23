#include <cassert>
#include <iostream>
#include <stdexcept>
#include "grafo.h"

void test_getters() {
    Grafo g(5, 6, 20);
    assert(g.getCantVert() == 5);
    assert(g.getMaxW() == 20);
    assert(g.getNodoDestino() == 4);
    std::cout << "test_getters: OK\n";
}

void test_insertarYGetVecinos() {
    Grafo g(3, 2, 10);
    g.insertarArista(0, 1, 2, 3);
    g.insertarArista(0, 2, 4, 1);

    auto vecinos = g.getVecinos(0);
    assert(vecinos.size() == 2);
    assert(vecinos[0].destino == 1 && vecinos[0].costo == 2 && vecinos[0].beneficio == 3);
    assert(vecinos[1].destino == 2 && vecinos[1].costo == 4 && vecinos[1].beneficio == 1);
    assert(g.getVecinos(1).empty());
    std::cout << "test_insertarYGetVecinos: OK\n";
}

void test_getArista() {
    Grafo g(3, 2, 10);
    g.insertarArista(0, 1, 2, 3);
    g.insertarArista(1, 2, 5, 4);

    Nodo n = g.getArista(0, 1);
    assert(n.costo == 2 && n.beneficio == 3);

    Nodo n2 = g.getArista(1, 2);
    assert(n2.costo == 5 && n2.beneficio == 4);
    std::cout << "test_getArista: OK\n";
}

void test_getAristaNoExiste() {
    Grafo g(3, 1, 10);
    g.insertarArista(0, 1, 2, 3);

    bool lanzo = false;
    try {
        g.getArista(0, 2);
    } catch (const std::runtime_error&) {
        lanzo = true;
    }
    assert(lanzo);
    std::cout << "test_getAristaNoExiste: OK\n";
}

void test_cargarDesdeArchivo() {
    Grafo g = Grafo::cargarDesdeArchivo("ejemplo.txt");
    assert(g.getCantVert() == 4);
    assert(g.getMaxW() == 7);
    assert(g.getNodoDestino() == 3);

    Nodo a01 = g.getArista(0, 1);
    assert(a01.costo == 1 && a01.beneficio == 4);

    Nodo a12 = g.getArista(1, 2);
    assert(a12.costo == 2 && a12.beneficio == 3);

    Nodo a23 = g.getArista(2, 3);
    assert(a23.costo == 3 && a23.beneficio == 2);

    Nodo a30 = g.getArista(3, 0);
    assert(a30.costo == 4 && a30.beneficio == 1);

    std::cout << "test_cargarDesdeArchivo: OK\n";
}

void test_dijkstra() {
    // 0->1 (1), 0->2 (4), 1->2 (2), 1->3 (5), 2->3 (1)
    // camino minimo 0->3: 0->1->2->3 = 4
    Grafo g(4, 5, 20);
    g.insertarArista(0, 1, 1, 0);
    g.insertarArista(0, 2, 4, 0);
    g.insertarArista(1, 2, 2, 0);
    g.insertarArista(1, 3, 5, 0);
    g.insertarArista(2, 3, 1, 0);

    auto dist = g.dijkstra(0);
    assert(dist[0] == 0);
    assert(dist[1] == 1);
    assert(dist[2] == 3);
    assert(dist[3] == 4);
    std::cout << "test_dijkstra: OK\n";
}

void test_dijkstraInvertido() {
    // mismo grafo, dijkstraInvertido desde 3
    // dist[v] = costo minimo de v hasta 3
    Grafo g(4, 5, 20);
    g.insertarArista(0, 1, 1, 0);
    g.insertarArista(0, 2, 4, 0);
    g.insertarArista(1, 2, 2, 0);
    g.insertarArista(1, 3, 5, 0);
    g.insertarArista(2, 3, 1, 0);

    auto dist = g.dijkstraInvertido(3);
    assert(dist[3] == 0);
    assert(dist[2] == 1);
    assert(dist[1] == 3); // 1->2->3
    assert(dist[0] == 4); // 0->1->2->3
    std::cout << "test_dijkstraInvertido: OK\n";
}

int main() {
    test_getters();
    test_insertarYGetVecinos();
    test_getArista();
    test_getAristaNoExiste();
    test_cargarDesdeArchivo();
    test_dijkstra();
    test_dijkstraInvertido();
    std::cout << "--- Todos los tests de Grafo pasaron ---\n";
    return 0;
}
