#include <cassert>
#include <iostream>
#include "algoritmo.h"

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

void test_permutar_k1() {
    auto res = Algoritmo::permutar({1, 2, 3}, 1);
    assert(res.size() == 3);
    assert(res[0] == (std::vector<int>{1}));
    assert(res[1] == (std::vector<int>{2}));
    assert(res[2] == (std::vector<int>{3}));
    std::cout << "test_permutar_k1: OK\n";
}

void test_permutar_k2_incluye_ambos_ordenes() {
    // a diferencia de combinar, (1,2) y (2,1) son resultados distintos
    auto res = Algoritmo::permutar({1, 2, 3}, 2);
    assert(res.size() == 6);
    assert(res[0] == (std::vector<int>{1, 2}));
    assert(res[1] == (std::vector<int>{1, 3}));
    assert(res[2] == (std::vector<int>{2, 1}));
    assert(res[3] == (std::vector<int>{2, 3}));
    assert(res[4] == (std::vector<int>{3, 1}));
    assert(res[5] == (std::vector<int>{3, 2}));
    std::cout << "test_permutar_k2_incluye_ambos_ordenes: OK\n";
}

void test_permutar_k_igual_candidatos() {
    auto res = Algoritmo::permutar({1, 2}, 2);
    assert(res.size() == 2);
    assert(res[0] == (std::vector<int>{1, 2}));
    assert(res[1] == (std::vector<int>{2, 1}));
    std::cout << "test_permutar_k_igual_candidatos: OK\n";
}

void test_permutar_k0() {
    auto res = Algoritmo::permutar({1, 2, 3}, 0);
    assert(res.size() == 1);
    assert(res[0].empty());
    std::cout << "test_permutar_k0: OK\n";
}

void test_permutar_k_mayor_que_candidatos() {
    auto res = Algoritmo::permutar({1, 2}, 3);
    assert(res.empty());
    std::cout << "test_permutar_k_mayor_que_candidatos: OK\n";
}

void test_permutar_candidatos_vacios() {
    auto res = Algoritmo::permutar({}, 1);
    assert(res.empty());
    std::cout << "test_permutar_candidatos_vacios: OK\n";
}

int main() {
    test_combinar_k1();
    test_combinar_k2();
    test_combinar_sin_duplicados();
    test_combinar_k_mayor_que_candidatos();
    test_combinar_candidatos_vacios();
    test_permutar_k1();
    test_permutar_k2_incluye_ambos_ordenes();
    test_permutar_k_igual_candidatos();
    test_permutar_k0();
    test_permutar_k_mayor_que_candidatos();
    test_permutar_candidatos_vacios();
    std::cout << "--- Todos los tests de Algoritmo pasaron ---\n";
    return 0;
}
