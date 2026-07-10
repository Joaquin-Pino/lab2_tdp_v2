#include <cassert>
#include <iostream>
#include <unordered_set>
#include "scatter.h"

using namespace std;

bool esCaminoValido(const Grafo& g, const vector<int>& camino) {
    if (camino.empty()) return false;
    if (camino.front() != g.getIdNodoInicial()) return false;
    if (camino.back() != g.getIdNodoFinal()) return false;

    unordered_set<int> vistos(camino.begin(), camino.end());
    return vistos.size() == camino.size(); // sin nodos repetidos
}

// Dos caminos independientes 0-1-2-5 y 0-3-4-5 que comparten solo los
// extremos. La arista 1->4 conecta la primera mitad de C1 con la segunda
// mitad de C2, asi que combinar() deberia unirlos por la Opcion A.
Grafo crearGrafoParaUnionA() {
    Grafo g(6, 7, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 5, 1, 1);
    g.insertarArista(0, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    g.insertarArista(4, 5, 1, 1);
    g.insertarArista(1, 4, 1, 5); // habilita la union: mitad1(C1)=[0,1] + mitad2(C2)=[4,5]
    return g;
}

void test_combinarUsaOpcionAConUnionValida() {
    Grafo g = crearGrafoParaUnionA();
    Camino c1(vector<int>{0, 1, 2, 5}, g);
    Camino c2(vector<int>{0, 3, 4, 5}, g);

    Scatter scatter(g);
    Camino resultado = scatter.combinar(c1, c2);

    assert((resultado.getCamino() == vector<int>{0, 1, 4, 5}));
    assert(resultado.getPesoTotal() == 3);
    assert(esCaminoValido(g, resultado.getCamino()));
    cout << "test_combinarUsaOpcionAConUnionValida: OK\n";
}

// Mismo par de caminos base, pero sin la arista 1->4: la Opcion A no puede
// unir. En cambio agregamos 3->2, que permite la Opcion B: primera mitad de
// C2 ([0,3]) + segunda mitad de C1 ([2,5]).
Grafo crearGrafoParaUnionB() {
    Grafo g(6, 6, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 5, 1, 1);
    g.insertarArista(0, 3, 1, 1);
    g.insertarArista(3, 4, 1, 1);
    g.insertarArista(4, 5, 1, 1);
    g.insertarArista(3, 2, 1, 5); // habilita solo la Opcion B
    return g;
}

void test_combinarUsaOpcionBCuandoAFalla() {
    Grafo g = crearGrafoParaUnionB();
    Camino c1(vector<int>{0, 1, 2, 5}, g);
    Camino c2(vector<int>{0, 3, 4, 5}, g);

    Scatter scatter(g);
    Camino resultado = scatter.combinar(c1, c2);

    assert((resultado.getCamino() == vector<int>{0, 3, 2, 5}));
    assert(resultado.getPesoTotal() == 3);
    assert(esCaminoValido(g, resultado.getCamino()));
    cout << "test_combinarUsaOpcionBCuandoAFalla: OK\n";
}

// Sin la arista 1->4 ni la 3->2, ninguna de las dos uniones tiene un punto
// de corte que exista como arista real: combinar() debe devolver el mejor
// de los dos padres tal cual (el de mayor beneficio).
void test_combinarRetornaMejorPadreSiNingunaUnionEsFactible() {
    Grafo g(6, 6, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 5, 1, 1);
    g.insertarArista(0, 3, 1, 1);
    g.insertarArista(3, 4, 1, 10); // C2 queda con mucho mas beneficio
    g.insertarArista(4, 5, 1, 1);

    Camino c1(vector<int>{0, 1, 2, 5}, g);
    Camino c2(vector<int>{0, 3, 4, 5}, g);
    assert(c2.getBeneficioTotal() > c1.getBeneficioTotal());

    Scatter scatter(g);
    Camino resultado = scatter.combinar(c1, c2);

    assert(resultado.getCamino() == c2.getCamino());
    cout << "test_combinarRetornaMejorPadreSiNingunaUnionEsFactible: OK\n";
}

// Mismo grafo que habilita la Opcion A (arista 1->4), pero con maxW=2: el
// camino combinado costaria 3, por lo que intentarUnion debe descartarlo
// por presupuesto (no por arista faltante) y caer al mejor padre.
void test_combinarDescartaUnionQueExcedePresupuesto() {
    Grafo g(6, 7, 2);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 2, 1, 1);
    g.insertarArista(2, 5, 1, 1);
    g.insertarArista(0, 3, 1, 1);
    g.insertarArista(3, 4, 1, 3); // C2 con mayor beneficio
    g.insertarArista(4, 5, 1, 1);
    g.insertarArista(1, 4, 1, 5);

    Camino c1(vector<int>{0, 1, 2, 5}, g);
    Camino c2(vector<int>{0, 3, 4, 5}, g);

    Scatter scatter(g);
    Camino resultado = scatter.combinar(c1, c2);

    assert(resultado.getCamino() == c2.getCamino());
    cout << "test_combinarDescartaUnionQueExcedePresupuesto: OK\n";
}

// Grafo denso (es el K4, N=4 -> densidad 1.0 >= 0.6): combinar debe usar la
// insercion de subcadena, no el empalme. base=[0,1,3] y donante=[0,2,3]
// comparten extremos; la arista {1,2} (alto beneficio) permite injertar el
// nodo 2 entre 1 y 3, dando 0-1-2-3 con mas beneficio que cualquiera de los
// dos padres.
// Al ser no dirigido el nodo 2 tambien entra en el hueco (0,1) dando 0-2-1-3,
// asi que {2,3} vale mas que {1,3} para que el injerto en (1,3) gane sin empate:
//   hueco (1,3): -b(1,3) + b(1,2) + b(2,3) = -1 + 5 + 2 = 6
//   hueco (0,1): -b(0,1) + b(0,2) + b(2,1) = -1 + 1 + 5 = 5
Grafo crearGrafoDenso() {
    Grafo g(4, 6, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(1, 3, 1, 1);
    g.insertarArista(0, 2, 1, 1);
    g.insertarArista(2, 3, 1, 2);
    g.insertarArista(1, 2, 1, 5); // habilita el injerto del nodo 2
    g.insertarArista(0, 3, 5, 1); // sube la densidad
    return g;
}

void test_combinarInsertaSubcadenaEnGrafoDenso() {
    Grafo g = crearGrafoDenso();
    Camino c1(vector<int>{0, 1, 3}, g);
    Camino c2(vector<int>{0, 2, 3}, g);

    Scatter scatter(g); // densidad 1.0 >= umbral 0.6 -> rama densa
    Camino resultado = scatter.combinar(c1, c2);

    assert((resultado.getCamino() == vector<int>{0, 1, 2, 3}));
    assert(resultado.getPesoTotal() == 3);
    assert(resultado.getBeneficioTotal() == 8); // 1 + 5 + 2
    assert(esCaminoValido(g, resultado.getCamino()));
    cout << "test_combinarInsertaSubcadenaEnGrafoDenso: OK\n";
}

// Grafo puramente lineal: el unico camino posible de 0 a N-1 es
// 0-1-2-3-4. Sin aristas alternativas, generarCandidatos() nunca encuentra
// nada que insertar, asi que la construccion aleatoria es en realidad
// deterministica y resolver() debe devolver exactamente ese camino.
Grafo crearGrafoLineal() {
    Grafo g(5, 4, 10);
    g.insertarArista(0, 1, 1, 2);
    g.insertarArista(1, 2, 1, 2);
    g.insertarArista(2, 3, 1, 2);
    g.insertarArista(3, 4, 1, 2);
    return g;
}

void test_resolverConCaminoUnicoDevuelveEseCamino() {
    Grafo g = crearGrafoLineal();
    Scatter scatter(g);

    Camino resultado = scatter.resolver(5);

    assert((resultado.getCamino() == vector<int>{0, 1, 2, 3, 4}));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 8);
    cout << "test_resolverConCaminoUnicoDevuelveEseCamino: OK\n";
}

// Sobre un grafo mas grande y con aristas alternativas, no hay un unico
// resultado esperado (construccionAleatoria usa el rng). Solo se verifican
// los invariantes que resolver() siempre debe cumplir: camino valido
// (arranca en 0, termina en N-1, sin nodos repetidos) y presupuesto
// respetado.
void test_resolverSobreGrafoGrandeRespetaInvariantes() {
    Grafo g = Grafo::cargarDesdeArchivo("grafoGrande.txt");
    Scatter scatter(g);

    Camino resultado = scatter.resolver(10);

    assert(esCaminoValido(g, resultado.getCamino()));
    assert(resultado.getPesoTotal() <= g.getMaxW());
    assert(resultado.getBeneficioTotal() > 0);
    cout << "test_resolverSobreGrafoGrandeRespetaInvariantes: OK\n";
}

int main() {
    test_combinarUsaOpcionAConUnionValida();
    test_combinarUsaOpcionBCuandoAFalla();
    test_combinarRetornaMejorPadreSiNingunaUnionEsFactible();
    test_combinarDescartaUnionQueExcedePresupuesto();
    test_combinarInsertaSubcadenaEnGrafoDenso();
    test_resolverConCaminoUnicoDevuelveEseCamino();
    test_resolverSobreGrafoGrandeRespetaInvariantes();
    cout << "--- Todos los tests de Scatter pasaron ---\n";
    return 0;
}
