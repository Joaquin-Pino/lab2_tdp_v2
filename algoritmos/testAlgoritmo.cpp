#include <cassert>
#include <iostream>
#include "algoritmo.h"
#include "../grafo/grafo.h"

// ===== combinar =====

void test_combinar_k1() {
    auto res = Algoritmo::combinar({10, 20, 30}, 1);
    assert(res.size() == 3);
    assert(res[0] == (std::vector<int>{10}));
    assert(res[1] == (std::vector<int>{20}));
    assert(res[2] == (std::vector<int>{30}));
    std::cout << "test_combinar_k1: OK\n";
}

void test_combinar_k2() {
    auto res = Algoritmo::combinar({1, 2, 3}, 2);
    assert(res.size() == 3);
    assert(res[0] == (std::vector<int>{1, 2}));
    assert(res[1] == (std::vector<int>{1, 3}));
    assert(res[2] == (std::vector<int>{2, 3}));
    std::cout << "test_combinar_k2: OK\n";
}

void test_combinar_sin_duplicados() {
    // (1,2) y (2,1) no deben aparecer ambos
    auto res = Algoritmo::combinar({1, 2}, 2);
    assert(res.size() == 1);
    assert(res[0] == (std::vector<int>{1, 2}));
    std::cout << "test_combinar_sin_duplicados: OK\n";
}

void test_combinar_k_mayor_que_candidatos() {
    auto res = Algoritmo::combinar({1, 2}, 3);
    assert(res.empty());
    std::cout << "test_combinar_k_mayor_que_candidatos: OK\n";
}

void test_combinar_candidatos_vacios() {
    auto res = Algoritmo::combinar({}, 1);
    assert(res.empty());
    std::cout << "test_combinar_candidatos_vacios: OK\n";
}

// ===== permutar =====
//
// Grafo de prueba (7 nodos, destino=6):
//   0->1, 1->2, 2->3, 3->6  (camino base)
//   0->5, 5->2               (nodo 5 encaja en hueco (0,2))
//   2->4, 4->6               (nodo 4 encaja en hueco (2,6))
//
Grafo crearGrafo() {
    Grafo g(7, 8, 20);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(3, 6, 1, 1);
    g.insertarArista(0, 5, 1, 2);
    g.insertarArista(5, 2, 1, 2);
    g.insertarArista(2, 4, 1, 2);
    g.insertarArista(4, 6, 1, 2);
    return g;
}

void test_permutar_k1_valido() {
    Grafo g = crearGrafo();
    // hueco (0,2): nodo 5 encaja (0->5 y 5->2 existen)
    auto res = Algoritmo::permutar({5}, {{0, 2}}, 1, g);
    assert(res.size() == 1);
    assert(res[0] == (std::vector<int>{5}));
    std::cout << "test_permutar_k1_valido: OK\n";
}

void test_permutar_k1_poda() {
    Grafo g = crearGrafo();
    // hueco (0,2): nodo 4 no encaja (no existe 0->4)
    auto res = Algoritmo::permutar({4}, {{0, 2}}, 1, g);
    assert(res.empty());
    std::cout << "test_permutar_k1_poda: OK\n";
}

void test_permutar_k1_varios_candidatos() {
    Grafo g = crearGrafo();
    // hueco (0,2): candidatos {4, 5}
    // 4: no existe 0->4 → podado
    // 5: existen 0->5 y 5->2 → valido
    auto res = Algoritmo::permutar({4, 5}, {{0, 2}}, 1, g);
    assert(res.size() == 1);
    assert(res[0] == (std::vector<int>{5}));
    std::cout << "test_permutar_k1_varios_candidatos: OK\n";
}

void test_permutar_k2_no_consecutivos() {
    Grafo g = crearGrafo();
    // Camino [0,1,2,3,6], sacar (1,3) no consecutivos
    // hueco[0]=(0,2): solo 5 encaja (0->5, 5->2)
    // hueco[1]=(2,6): solo 4 encaja (2->4, 4->6)
    // unica permutacion valida: (5, 4)
    auto res = Algoritmo::permutar({4, 5}, {{0, 2}, {2, 6}}, 2, g);
    assert(res.size() == 1);
    assert(res[0] == (std::vector<int>{5, 4}));
    std::cout << "test_permutar_k2_no_consecutivos: OK\n";
}

void test_permutar_sin_candidatos_validos() {
    Grafo g = crearGrafo();
    // hueco (0,6): ningun candidato tiene aristas directas 0->X y X->6
    auto res = Algoritmo::permutar({1, 2, 3}, {{0, 6}}, 1, g);
    assert(res.empty());
    std::cout << "test_permutar_sin_candidatos_validos: OK\n";
}

int main() {
    test_combinar_k1();
    test_combinar_k2();
    test_combinar_sin_duplicados();
    test_combinar_k_mayor_que_candidatos();
    test_combinar_candidatos_vacios();
    test_permutar_k1_valido();
    test_permutar_k1_poda();
    test_permutar_k1_varios_candidatos();
    test_permutar_k2_no_consecutivos();
    test_permutar_sin_candidatos_validos();
    std::cout << "--- Todos los tests de Algoritmo pasaron ---\n";
    return 0;
}
