#include <cassert>
#include <iostream>

#include "branchAndBound.h"
#include "../grafo/grafo.h"
#include "../camino/camino.h"

using namespace std;

// Grafo chico donde el optimo se conoce a mano.
//   0 -> 1 (c1,b1)   0 -> 2 (c1,b5)
//   1 -> 3 (c1,b1)   2 -> 3 (c1,b1)   1 -> 2 (c1,b5)
//   destino = 3, W = 10
// El mejor camino es 0->1->2->3 (peso 3, beneficio 1+5+1=7) frente a 0->2->3 (b6).
static Grafo construirGrafo() {
    Grafo g(4, 5, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(0, 2, 1, 5);
    g.insertarArista(1, 3, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(1, 2, 1, 5);
    return g;
}

void test_encuentraOptimo() {
    Grafo g = construirGrafo();
    SolverBranchAndBound solver(g, 1000000);
    Camino r = solver.resolver();

    assert(r.esCaminoCompleto());
    assert(r.getPesoTotal() <= g.getMaxW());
    // el optimo alcanzable es 0->1->2->3 con beneficio 7
    assert(r.getBeneficioTotal() == 7);
    vector<int> esperado = {0, 1, 2, 3};
    assert(r.getCamino() == esperado);
    cout << "test_encuentraOptimo OK\n";
}

void test_respetaPresupuesto() {
    // mismo grafo pero W apretado: solo alcanza para un camino de 2 aristas
    Grafo g(4, 5, 2);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(0, 2, 1, 5);
    g.insertarArista(1, 3, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    g.insertarArista(1, 2, 1, 5);

    SolverBranchAndBound solver(g, 1000000);
    Camino r = solver.resolver();

    assert(r.esCaminoCompleto());
    assert(r.getPesoTotal() <= 2);
    // el mejor de 2 aristas es 0->2->3 (beneficio 6)
    assert(r.getBeneficioTotal() == 6);
    cout << "test_respetaPresupuesto OK\n";
}

void test_limiteIteraciones() {
    // con un tope muy bajo igual debe devolver un camino completo (el de las
    // heuristicas), nunca uno invalido.
    Grafo g = construirGrafo();
    SolverBranchAndBound solver(g, 1);
    Camino r = solver.resolver();

    assert(r.esCaminoCompleto());
    assert(r.getPesoTotal() <= g.getMaxW());
    cout << "test_limiteIteraciones OK\n";
}

int main() {
    test_encuentraOptimo();
    test_respetaPresupuesto();
    test_limiteIteraciones();
    cout << "Todos los tests de BranchAndBound OK\n";
    return 0;
}
