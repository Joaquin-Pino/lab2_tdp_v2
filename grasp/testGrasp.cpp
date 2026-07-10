#include <cassert>
#include <iostream>
#include <unordered_set>
#include "grasp.h"

using namespace std;

bool esCaminoValido(const Grafo& g, const vector<int>& camino) {
    if (camino.empty()) return false;
    if (camino.front() != g.getIdNodoInicial()) return false;
    if (camino.back() != g.getIdNodoFinal()) return false;

    unordered_set<int> vistos(camino.begin(), camino.end());
    return vistos.size() == camino.size(); // sin nodos repetidos
}

// Grafo puramente lineal: el unico camino posible de 0 a N-1 es 0-1-2-3-4.
// Sin aristas alternativas, la RCL siempre tiene un solo candidato, asi que
// construir() es en realidad deterministico.
Grafo crearGrafoLineal() {
    Grafo g(5, 4, 10);
    g.insertarArista(0, 1, 1, 2);
    g.insertarArista(1, 2, 1, 2);
    g.insertarArista(2, 3, 1, 2);
    g.insertarArista(3, 4, 1, 2);
    return g;
}

void test_construirConCaminoUnicoDevuelveEseCamino() {
    Grafo g = crearGrafoLineal();
    Grasp grasp(g);

    Camino resultado = grasp.construir();

    assert((resultado.getCamino() == vector<int>{0, 1, 2, 3, 4}));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 8);
    cout << "test_construirConCaminoUnicoDevuelveEseCamino: OK\n";
}

// El corte de la RCL es peorEf + alpha*(mejorEf-peorEf): con alpha=1 el corte
// queda en mejorEf, asi que la RCL se reduce a los candidatos empatados en la
// mejor eficiencia -> construir() se vuelve goloso puro y deterministico.
// Desde 0: 0-1 (ef=1) vs 0-2 (ef=5) -> RCL={2}. Desde 2 (sin volver a 0):
// 2-1 (ef=5) vs 2-3 (ef=1) -> RCL={1}. Desde 1 (sin volver a 0,2): solo
// queda 1-3 (ef=1). Camino resultante: 0-2-1-3.
Grafo crearGrafoConRamaDominante() {
    Grafo g(4, 5, 10);
    g.insertarArista(0, 1, 1, 1);
    g.insertarArista(0, 2, 1, 5);
    g.insertarArista(1, 2, 1, 5);
    g.insertarArista(1, 3, 1, 1);
    g.insertarArista(2, 3, 1, 1);
    return g;
}

void test_construirConAlphaUnoEsGolosoPuro() {
    Grafo g = crearGrafoConRamaDominante();
    Grasp grasp(g, 1.0);

    Camino resultado = grasp.construir();

    assert(esCaminoValido(g, resultado.getCamino()));
    assert((resultado.getCamino() == vector<int>{0, 2, 1, 3}));
    assert(resultado.getBeneficioTotal() == 11); // 5 + 5 + 1
    cout << "test_construirConAlphaUnoEsGolosoPuro: OK\n";
}

// Sobre un grafo mas grande no hay un unico resultado esperado (construir()
// usa el rng). Solo se verifican los invariantes que siempre deben cumplirse.
void test_construirSobreGrafoGrandeRespetaInvariantes() {
    Grafo g = Grafo::cargarDesdeArchivo("grafoGrande.txt");
    Grasp grasp(g);

    Camino resultado = grasp.construir();

    assert(esCaminoValido(g, resultado.getCamino()));
    assert(resultado.getPesoTotal() <= g.getMaxW());
    assert(resultado.getBeneficioTotal() > 0);
    cout << "test_construirSobreGrafoGrandeRespetaInvariantes: OK\n";
}

void test_generarPoblacionDevuelveCaminosValidosYRefinados() {
    Grafo g = Grafo::cargarDesdeArchivo("grafoGrande.txt");
    Grasp grasp(g);

    vector<Camino> poblacion = grasp.generarPoblacion(10);

    assert(poblacion.size() == 10);
    for (Camino& c : poblacion) {
        assert(esCaminoValido(g, c.getCamino()));
        assert(c.getPesoTotal() <= g.getMaxW());
    }
    cout << "test_generarPoblacionDevuelveCaminosValidosYRefinados: OK\n";
}

// resolver() como metaheuristica standalone: sobre el grafo lineal el
// resultado es siempre el mismo (unico camino posible), y debe respetar el
// presupuesto y llegar al destino igual que construir().
void test_resolverConCaminoUnicoDevuelveEseCamino() {
    Grafo g = crearGrafoLineal();
    Grasp grasp(g);

    Camino resultado = grasp.resolver(5);

    assert((resultado.getCamino() == vector<int>{0, 1, 2, 3, 4}));
    assert(resultado.getPesoTotal() == 4);
    assert(resultado.getBeneficioTotal() == 8);
    cout << "test_resolverConCaminoUnicoDevuelveEseCamino: OK\n";
}

// resolver() nunca debe devolver algo peor que una sola construccion+refine:
// con mas iteraciones el beneficio del mejor encontrado no puede bajar.
void test_resolverNoEmpeoraAlIterarMas() {
    Grafo g = Grafo::cargarDesdeArchivo("grafoGrande.txt");
    Grasp grasp(g);

    Camino conUnaIter = grasp.resolver(1);
    Camino conVariasIter = grasp.resolver(15);

    assert(esCaminoValido(g, conVariasIter.getCamino()));
    assert(conVariasIter.getPesoTotal() <= g.getMaxW());
    assert(conVariasIter.getBeneficioTotal() >= conUnaIter.getBeneficioTotal());
    cout << "test_resolverNoEmpeoraAlIterarMas: OK\n";
}

int main() {
    test_construirConCaminoUnicoDevuelveEseCamino();
    test_construirConAlphaUnoEsGolosoPuro();
    test_construirSobreGrafoGrandeRespetaInvariantes();
    test_generarPoblacionDevuelveCaminosValidosYRefinados();
    test_resolverConCaminoUnicoDevuelveEseCamino();
    test_resolverNoEmpeoraAlIterarMas();
    cout << "--- Todos los tests de Grasp pasaron ---\n";
    return 0;
}
