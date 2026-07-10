#include <cassert>
#include <iostream>
#include "camino.h"

// Grafo de prueba (no dirigido):
// 0-1 (costo=2, beneficio=3)
// 0-2 (costo=4, beneficio=1)
// 1-2 (costo=1, beneficio=2)
// 1-3 (costo=3, beneficio=5)
// 2-3 (costo=2, beneficio=4)
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
    // 0-1-2-3: pesoTotal=2+1+2=5, beneficioTotal=3+2+4=9

    assert(c.eliminarNodo(2));
    // queda 0-1-3: saca {1,2}(1,2) y {2,3}(2,4), y suma la arista puente {1,3}(3,5)
    // peso      = 5 + 3 - 1 - 2 = 5
    // beneficio = 9 + 5 - 2 - 4 = 8
    assert((c.getCamino() == std::vector<int>{0, 1, 3}));
    assert(c.getPesoTotal() == 5);
    assert(c.getBeneficioTotal() == 8);
    assert(!c.nodoFueVisitado(2));
    std::cout << "test_eliminarNodo: OK\n";
}

// Los totales tras eliminar deben coincidir con recalcular el camino desde cero.
void test_eliminarNodoMantieneTotalesConsistentes() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{0, 1, 2, 3}, g);

    assert(c.eliminarNodo(2));

    Camino esperado(std::vector<int>{0, 1, 3}, g);
    assert(c.getPesoTotal() == esperado.getPesoTotal());
    assert(c.getBeneficioTotal() == esperado.getBeneficioTotal());
    std::cout << "test_eliminarNodoMantieneTotalesConsistentes: OK\n";
}

// Los extremos y los nodos ausentes no se eliminan, y el camino queda intacto.
void test_eliminarNodoRechazaExtremosYAusentes() {
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{0, 1, 2, 3}, g);

    assert(!c.eliminarNodo(0));  // primero
    assert(!c.eliminarNodo(3));  // ultimo
    assert(!c.eliminarNodo(99)); // no esta en el camino

    assert((c.getCamino() == std::vector<int>{0, 1, 2, 3}));
    assert(c.getPesoTotal() == 5);
    assert(c.getBeneficioTotal() == 9);
    assert(c.nodoFueVisitado(0) && c.nodoFueVisitado(3));
    std::cout << "test_eliminarNodoRechazaExtremosYAusentes: OK\n";
}

// Si al sacar el nodo sus vecinos no quedan unidos por una arista real, la
// eliminacion se rechaza: el camino resultante no existiria en el grafo.
void test_eliminarNodoRechazaSiNoHayAristaPuente() {
    Grafo g(4, 3, 10); // lineal: 0-1-2-3, sin atajos
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 3, 1, 1);

    Camino c(std::vector<int>{0, 1, 2, 3}, g);
    assert(!g.existeArista(0, 2));

    assert(!c.eliminarNodo(1)); // 0 y 2 no son adyacentes

    assert((c.getCamino() == std::vector<int>{0, 1, 2, 3}));
    assert(c.getPesoTotal() == 3);
    assert(c.getBeneficioTotal() == 3);
    assert(c.nodoFueVisitado(1));
    std::cout << "test_eliminarNodoRechazaSiNoHayAristaPuente: OK\n";
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
    // camino 0-1-2-3, swap 1 y 2 -> 0-2-1-3.
    // El grafo es no dirigido, asi que la arista {1,2} sirve en ambos sentidos
    // y el swap es valido: solo hace falta que existan {0,2} y {1,3}.
    Grafo g = crearGrafo();

    Camino c(std::vector<int>{}, g);
    c.agregarNodo(0);
    c.agregarNodo(1);
    c.agregarNodo(2);
    c.agregarNodo(3);
    // 0-1-2-3: pesoTotal=2+1+2=5, beneficioTotal=3+2+4=9

    c.intercambiarNodos(1, 2);
    // 0-2-1-3: pesoTotal=4+1+3=8, beneficioTotal=1+2+5=8
    assert(c.getPesoTotal() == 8);
    assert(c.getBeneficioTotal() == 8);
    std::cout << "test_intercambiarNodos: OK\n";
}

void test_concatenar() {
    // simula el uso real: dijkstraCamino(origen, destino) devuelve
    // [origen, ..., destino], donde origen es el ultimo nodo ya presente
    // en el camino (p.ej. al completar el camino del goloso).
    Grafo g = crearGrafo();
    Camino c(std::vector<int>{}, g);
    c.agregarNodo(0);
    c.agregarNodo(1);
    // pesoTotal=2, beneficioTotal=3

    c.concatenar(std::vector<int>{1, 3}); // 1 ya esta en el camino

    assert((c.getCamino() == std::vector<int>{0, 1, 3})); // sin duplicar el 1
    assert(c.getPesoTotal() == 5); // 2 + arista(1->3)=3
    assert(c.getBeneficioTotal() == 8); // 3 + arista(1->3)=5
    assert(c.nodoFueVisitado(3));
    std::cout << "test_concatenar: OK\n";
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
    test_eliminarNodoMantieneTotalesConsistentes();
    test_eliminarNodoRechazaExtremosYAusentes();
    test_eliminarNodoRechazaSiNoHayAristaPuente();
    test_verificarCamino();
    test_getUltimoNodo();
    test_intercambiarNodos();
    test_concatenar();
    test_copiaCamino();
    std::cout << "--- Todos los tests de Camino pasaron ---\n";
    return 0;
}
