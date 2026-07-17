#include <cassert>
#include <iostream>
#include <algorithm>

#include "planner.h"
#include "../grafo/grafo.h"

using namespace std;

class TestPlanner {
public:
    // True si el plan contiene un paso con esa heuristica.
    static bool contiene(const PlanCota& plan, PasoCota::Heuristica h) {
        for (const PasoCota& p : plan.pasos)
            if (p.heuristica == h) return true;
        return false;
    }

    // Grafo chico (4 nodos) denso: el plan debe ir a portafolio Scatter + GRASP,
    // siempre con Greedy de piso.
    static void test_grafoChico() {
        Grafo g(4, 5, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(0, 2, 1, 5);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(1, 2, 1, 5);

        Planner planner(g);
        PlanCota plan = planner.planificar();

        assert(!plan.pasos.empty());
        // Greedy siempre primero como piso universal.
        assert(plan.pasos.front().heuristica == PasoCota::Heuristica::GREEDY);
        assert(contiene(plan, PasoCota::Heuristica::GREEDY));
        assert(contiene(plan, PasoCota::Heuristica::SCATTER));
        assert(contiene(plan, PasoCota::Heuristica::GRASP));
        cout << "test_grafoChico OK\n";
    }

    // Grafo grande sintetico (2000 nodos en cadena): el plan debe usar solo GRASP
    // (mas Greedy), nunca Scatter, y con iteraciones positivas.
    static void test_grafoGrande() {
        const int n = 2000;
        Grafo g(n, n - 1, 100000);
        for (int i = 0; i + 1 < n; ++i)
            g.insertarArista(i, i + 1, 1, 1);

        Planner planner(g);
        PlanCota plan = planner.planificar();

        assert(plan.pasos.front().heuristica == PasoCota::Heuristica::GREEDY);
        assert(contiene(plan, PasoCota::Heuristica::GRASP));
        assert(!contiene(plan, PasoCota::Heuristica::SCATTER));
        cout << "test_grafoGrande OK\n";
    }

    // Todos los pasos que corren una metaheuristica (Scatter/GRASP/Breakout)
    // deben traer iteraciones > 0 y parametros en rango; Greedy no las usa.
    static void test_parametrosEnRango() {
        Grafo g(4, 5, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(0, 2, 1, 5);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(1, 2, 1, 5);

        Planner planner(g);
        PlanCota plan = planner.planificar();

        for (const PasoCota& p : plan.pasos) {
            if (p.heuristica == PasoCota::Heuristica::SCATTER) {
                assert(p.iteraciones > 0);
                assert(p.tamPoblacion > 0);
                assert(p.tamRefSet > 0);
                assert(p.tamRefSet <= p.tamPoblacion);
            }
            if (p.heuristica == PasoCota::Heuristica::GRASP) {
                assert(p.iteraciones > 0);
                assert(p.alpha >= 0.0 && p.alpha <= 1.0);
                assert(p.umbralRefine > 0);
            }
        }
        cout << "test_parametrosEnRango OK\n";
    }

    // El plan es determinista: mismo grafo -> mismo plan (el Planner es puro).
    static void test_esDeterminista() {
        Grafo g(4, 5, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(0, 2, 1, 5);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(1, 2, 1, 5);

        Planner planner(g);
        PlanCota a = planner.planificar();
        PlanCota b = planner.planificar();

        assert(a.pasos.size() == b.pasos.size());
        for (size_t i = 0; i < a.pasos.size(); ++i)
            assert(a.pasos[i].heuristica == b.pasos[i].heuristica);
        cout << "test_esDeterminista OK\n";
    }
};

int main() {
    TestPlanner::test_grafoChico();
    TestPlanner::test_grafoGrande();
    TestPlanner::test_parametrosEnRango();
    TestPlanner::test_esDeterminista();
    cout << "Todos los tests de Planner OK\n";
    return 0;
}
