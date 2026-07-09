#include <cassert>
#include <iostream>
#include <unordered_set>
#include "breakout.h"
#include "../solverGreedy/solverGreedy.h"

using namespace std;

// Mismo grafo que testKopt: 0-1-2-3-4 es el camino directo, pero
// 0-2-1-3-4 usa aristas alternativas de mejor beneficio.
// Camino directo: peso=4, beneficio=4. Con el swap: peso=4, beneficio=6.
// El goloso ya encuentra este optimo solo, asi que sirve para verificar
// que Breakout no lo empeora (caminoBest solo se actualiza si mejora).
Grafo crearGrafoConMejoraSwap() {
    Grafo g(5, 6, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    g.insertarArista(0, 2, 1, 2);
    g.insertarArista(1, 3, 1, 2);
    return g;
}

bool esCaminoValido(const Grafo& g, const vector<int>& camino) {
    if (camino.empty()) return false;
    if (camino.front() != g.getIdNodoInicial()) return false;
    if (camino.back() != g.getIdNodoFinal()) return false;

    unordered_set<int> vistos(camino.begin(), camino.end());
    return vistos.size() == camino.size(); // sin nodos repetidos
}

void test_resolverEncuentraOptimoConocido() {
    Grafo g = crearGrafoConMejoraSwap();
    Breakout breakout(g, 20);

    Camino resultado = breakout.resolver();

    assert(esCaminoValido(g, resultado.getCamino()));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 6);
    assert(resultado.getPesoTotal() <= g.getMaxW());
    cout << "test_resolverEncuentraOptimoConocido: OK\n";
}

// Con maxIter=0 el while de resolver() no itera ninguna vez, asi que debe
// devolver exactamente la solucion inicial (goloso, ya completa en este
// grafo) sin ninguna perturbacion ni refinamiento.
void test_resolverConCeroIteracionesNoCambiaSolucionInicial() {
    Grafo g = crearGrafoConMejoraSwap();
    Breakout breakout(g, 0);

    Camino resultado = breakout.resolver();

    SolverGreedy greedy(g);
    Camino esperado = greedy.resolver();
    assert(esperado.llegaFinal()); // en este grafo el goloso ya llega al final

    assert(resultado.getCamino() == esperado.getCamino());
    assert(resultado.getPesoTotal() == esperado.getPesoTotal());
    assert(resultado.getBeneficioTotal() == esperado.getBeneficioTotal());
    cout << "test_resolverConCeroIteracionesNoCambiaSolucionInicial: OK\n";
}

// Sobre un grafo mas grande, Breakout parte de la misma solucion goloso
// que SolverGreedy y solo actualiza caminoBest cuando encuentra algo mejor,
// asi que nunca deberia entregar un resultado peor que el goloso solo, y
// el camino final debe seguir siendo valido y respetar el presupuesto.
void test_resolverNuncaEmpeoraElGolosoYRespetaPresupuesto() {
    Grafo g = Grafo::cargarDesdeArchivo("grafoGrande.txt");

    SolverGreedy greedy(g);
    Camino base = greedy.resolver();
    assert(base.llegaFinal());

    Breakout breakout(g, 30);
    Camino resultado = breakout.resolver();

    assert(esCaminoValido(g, resultado.getCamino()));
    assert(resultado.getPesoTotal() <= g.getMaxW());
    assert(resultado.getBeneficioTotal() >= base.getBeneficioTotal());
    cout << "test_resolverNuncaEmpeoraElGolosoYRespetaPresupuesto: OK\n";
}

int main() {
    test_resolverEncuentraOptimoConocido();
    test_resolverConCeroIteracionesNoCambiaSolucionInicial();
    test_resolverNuncaEmpeoraElGolosoYRespetaPresupuesto();
    cout << "--- Todos los tests de Breakout pasaron ---\n";
    return 0;
}
