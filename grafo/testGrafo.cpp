#include <cassert>
#include <iostream>
#include <stdexcept>
#include "grafo.h"

class TestGrafo {
public:

    static void test_getters() {
        Grafo g(5, 6, 20);
        assert(g.getCantVert() == 5);
        assert(g.getMaxW() == 20);
        assert(g.getNodoDestino() == 4);
        std::cout << "test_getters: OK\n";
    }

    static void test_insertarYGetVecinos() {
        Grafo g(3, 2, 10);
        g.insertarArista(0, 1, 2, 3);
        g.insertarArista(0, 2, 4, 1);

        auto vecinos = g.getVecinos(0);
        assert(vecinos.size() == 2);
        assert(vecinos[0].destino == 1 && vecinos[0].costo == 2 && vecinos[0].beneficio == 3);
        assert(vecinos[1].destino == 2 && vecinos[1].costo == 4 && vecinos[1].beneficio == 1);

        // grafo no dirigido: cada extremo ve a la arista con el mismo costo/beneficio
        auto vecinos1 = g.getVecinos(1);
        assert(vecinos1.size() == 1);
        assert(vecinos1[0].destino == 0 && vecinos1[0].costo == 2 && vecinos1[0].beneficio == 3);

        auto vecinos2 = g.getVecinos(2);
        assert(vecinos2.size() == 1);
        assert(vecinos2[0].destino == 0 && vecinos2[0].costo == 4 && vecinos2[0].beneficio == 1);
        std::cout << "test_insertarYGetVecinos: OK\n";
    }

    // existeArista y getArista deben ser simetricas: la arista {0,1} se consulta
    // indistintamente en cualquiera de los dos sentidos.
    static void test_aristaEsSimetrica() {
        Grafo g(3, 1, 10);
        g.insertarArista(0, 1, 2, 3);

        assert(g.existeArista(0, 1));
        assert(g.existeArista(1, 0));
        assert(!g.existeArista(0, 2));
        assert(!g.existeArista(2, 0));

        assert(g.getArista(0, 1).costo == g.getArista(1, 0).costo);
        assert(g.getArista(0, 1).beneficio == g.getArista(1, 0).beneficio);
        assert(g.getPeso(1, 0) == 2 && g.getBeneficio(1, 0) == 3);
        std::cout << "test_aristaEsSimetrica: OK\n";
    }

    // Una arista repetida (en el mismo sentido o en el inverso, aun con otro
    // costo/beneficio) se ignora: se conserva la primera lectura. Sin esto
    // listaAdy tendria aristas paralelas que ady no puede representar.
    static void test_insertarAristaDuplicadaSeIgnora() {
        Grafo g(3, 3, 10);
        g.insertarArista(0, 1, 2, 3);
        g.insertarArista(0, 1, 9, 9); // duplicada, mismo sentido
        g.insertarArista(1, 0, 7, 7); // duplicada, sentido inverso

        assert(g.getVecinos(0).size() == 1);
        assert(g.getVecinos(1).size() == 1);
        assert(g.getPeso(0, 1) == 2 && g.getBeneficio(0, 1) == 3);
        assert(g.getPeso(1, 0) == 2 && g.getBeneficio(1, 0) == 3);
        std::cout << "test_insertarAristaDuplicadaSeIgnora: OK\n";
    }

    // Un lazo no puede formar parte de un camino simple: se descarta al insertar.
    static void test_insertarLazoSeIgnora() {
        Grafo g(3, 1, 10);
        g.insertarArista(1, 1, 5, 5);

        assert(g.getVecinos(1).empty());
        assert(!g.existeArista(1, 1));
        std::cout << "test_insertarLazoSeIgnora: OK\n";
    }

    static void test_getArista() {
        Grafo g(3, 2, 10);
        g.insertarArista(0, 1, 2, 3);
        g.insertarArista(1, 2, 5, 4);

        Nodo n = g.getArista(0, 1);
        assert(n.costo == 2 && n.beneficio == 3);

        Nodo n2 = g.getArista(1, 2);
        assert(n2.costo == 5 && n2.beneficio == 4);
        std::cout << "test_getArista: OK\n";
    }

    static void test_getAristaNoExiste() {
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

    static void test_cargarDesdeArchivo() {
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

        // las aristas del archivo se leen como no dirigidas
        assert(g.existeArista(1, 0) && g.existeArista(0, 3));
        assert(g.getPeso(0, 3) == 4 && g.getBeneficio(0, 3) == 1);

        std::cout << "test_cargarDesdeArchivo: OK\n";
    }

    static void test_dijkstra() {
        // 0-1 (1), 0-2 (4), 1-2 (2), 1-3 (5), 2-3 (1)
        // camino minimo 0..3: 0-1-2-3 = 4
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

    static void test_dijkstraInvertido() {
        // mismo grafo, dijkstraInvertido desde 3
        // dist[v] = costo minimo de v hasta 3 (la cota que usa la poda de B&B)
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
};

int main() {
    TestGrafo::test_getters();
    TestGrafo::test_insertarYGetVecinos();
    TestGrafo::test_aristaEsSimetrica();
    TestGrafo::test_insertarAristaDuplicadaSeIgnora();
    TestGrafo::test_insertarLazoSeIgnora();
    TestGrafo::test_getArista();
    TestGrafo::test_getAristaNoExiste();
    TestGrafo::test_cargarDesdeArchivo();
    TestGrafo::test_dijkstra();
    TestGrafo::test_dijkstraInvertido();
    std::cout << "--- Todos los tests de Grafo pasaron ---\n";
    return 0;
}
