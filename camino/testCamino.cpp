#include <cassert>
#include <iostream>
#include "camino.hpp"

// Grafo de prueba:
// 0->1 (costo=2, beneficio=3)
// 0->2 (costo=4, beneficio=1)
// 1->2 (costo=1, beneficio=2)
// 1->3 (costo=3, beneficio=5)
// 2->3 (costo=2, beneficio=4)
// maxW=10, destino=3

Grafo crearGrafo() {
    Grafo g(4, 5, 10);
    g.insertarArista(0, 1, 2, 3);
    g.insertarArista(0, 2, 4, 1);
    g.insertarArista(1, 2, 1, 2);
    g.insertarArista(1, 3, 3, 5);
    g.insertarArista(2, 3, 2, 4);
    return g;
}

void test_agregarNodo() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{}, g);

    c.agregarNodo(0);
    assert(c.getPesoTotal() == 0 && c.getBeneficioTotal() == 0);

    c.agregarNodo(1);
    assert(c.getPesoTotal() == 2 && c.getBeneficioTotal() == 3);

    c.agregarNodo(3);
    assert(c.getPesoTotal() == 5 && c.getBeneficioTotal() == 8);
    std::cout << "test_agregarNodo: OK\n";
}

void test_agregarNodoDuplicado() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{}, g);

    c.agregarNodo(0);
    c.agregarNodo(1);
    c.agregarNodo(1); // no debe agregarse de nuevo

    assert(c.getPesoTotal() == 2);
    assert(c.getCamino().size() == 2);
    std::cout << "test_agregarNodoDuplicado: OK\n";
}

void test_nodoFueVisitado() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{}, g);

    assert(!c.nodoFueVisitado(0));
    c.agregarNodo(0);
    assert(c.nodoFueVisitado(0));
    assert(!c.nodoFueVisitado(1));
    std::cout << "test_nodoFueVisitado: OK\n";
}

void test_eliminarNodo() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{}, g);

    c.agregarNodo(0);
    c.agregarNodo(1);
    c.agregarNodo(2);
    c.agregarNodo(3);
    // 0->1->2->3: pesoTotal=2+1+2=5, beneficioTotal=3+2+4=9

    c.eliminarNodo(2);
    // resta arista(1->2): costo=1, beneficio=2
    // resta arista(2->3): costo=2, beneficio=4
    assert(c.getPesoTotal() == 2);
    assert(c.getBeneficioTotal() == 3);
    assert(!c.nodoFueVisitado(2));
    assert(c.getCamino().size() == 3);
    std::cout << "test_eliminarNodo: OK\n";
}

void test_verificarCamino() {
    Grafo g = crearGrafo(); // maxW=10
    Camino c(std::vector<int>{}, g);

    c.agregarNodo(0);
    c.agregarNodo(1);
    c.agregarNodo(3); // pesoTotal=5

    assert(c.verificarCamino(10));
    assert(c.verificarCamino(5));
    assert(!c.verificarCamino(4));
    std::cout << "test_verificarCamino: OK\n";
}

void test_getUltimoNodo() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{}, g);

    c.agregarNodo(0);
    assert(c.getUltimoNodo() == 0);
    c.agregarNodo(1);
    assert(c.getUltimoNodo() == 1);
    c.agregarNodo(2);
    assert(c.getUltimoNodo() == 2);
    std::cout << "test_getUltimoNodo: OK\n";
}

void test_intercambiarNodos() {
    // camino 0->1->2->3, swap 1 y 2 -> 0->2->1->3
    // necesita arista 2->1 para que el swap sea valido
    Grafo g(4, 6, 20);
    g.insertarArista(0, 1, 2, 3);
    g.insertarArista(0, 2, 4, 1);
    g.insertarArista(1, 2, 1, 2);
    g.insertarArista(2, 1, 3, 2);
    g.insertarArista(1, 3, 3, 5);
    g.insertarArista(2, 3, 2, 4);

    Camino c(std::vector<int>{}, g);
    c.agregarNodo(0);
    c.agregarNodo(1);
    c.agregarNodo(2);
    c.agregarNodo(3);
    // 0->1->2->3: pesoTotal=2+1+2=5, beneficioTotal=3+2+4=9

    c.intercambiarNodos(1, 2);
    // 0->2->1->3: pesoTotal=4+3+3=10, beneficioTotal=1+2+5=8
    assert(c.getPesoTotal() == 10);
    assert(c.getBeneficioTotal() == 8);
    std::cout << "test_intercambiarNodos: OK\n";
}

void test_copiaCamino() {
    Grafo g = crearGrafo();
    Camino c1(std::vector<int>{}, g);
    c1.agregarNodo(0);
    c1.agregarNodo(1);

    Camino c2 = c1;
    c2.agregarNodo(3);

    assert(c1.getCamino().size() == 2);
    assert(c2.getCamino().size() == 3);
    assert(c2.getPesoTotal() == 5 && c2.getBeneficioTotal() == 8);
    std::cout << "test_copiaCamino: OK\n";
}

int main() {
    test_agregarNodo();
    test_agregarNodoDuplicado();
    test_nodoFueVisitado();
    test_eliminarNodo();
    test_verificarCamino();
    test_getUltimoNodo();
    test_intercambiarNodos();
    test_copiaCamino();
    std::cout << "--- Todos los tests de Camino pasaron ---\n";
    return 0;
}
