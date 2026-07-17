#include <cassert>
#include <iostream>

#include "planner.h"
#include "../grafo/grafo.h"

using namespace std;

class TestPlanner {
public:
    static bool contiene(const PlanCota& plan, PasoCota::Heuristica h) {
        for (const PasoCota& p : plan.pasos)
            if (p.heuristica == h) return true;
        return false;
    }

    static const PasoCota* pasoDe(const PlanCota& plan, PasoCota::Heuristica h) {
        for (const PasoCota& p : plan.pasos)
            if (p.heuristica == h) return &p;
        return nullptr;
    }

    // Grafo chico (4 nodos, grado medio 2.5).
    static Grafo grafoChico() {
        Grafo g(4, 5, 10);
        g.insertarArista(0, 1, 1, 1);
        g.insertarArista(0, 2, 1, 5);
        g.insertarArista(1, 3, 1, 1);
        g.insertarArista(2, 3, 1, 1);
        g.insertarArista(1, 2, 1, 5);
        return g;
    }

    // Camino final corto => Scatter es asequible => aparece en el plan (ademas de
    // GRASP, que va siempre).
    static void test_scatterAsequibleCaminoCorto() {
        Grafo g = grafoChico();
        Planner planner(g);
        PlanCota plan = planner.planificar(Sonda{/*largoFinal*/ 10, /*holgura*/ 0.3});

        assert(contiene(plan, PasoCota::Heuristica::SCATTER));
        assert(contiene(plan, PasoCota::Heuristica::GRASP));
        cout << "test_scatterAsequibleCaminoCorto OK\n";
    }

    // Mismo grafo pero con un camino final largo: L^2*grado supera el umbral =>
    // Scatter deja de ser asequible => el plan usa solo GRASP.
    static void test_scatterNoAsequibleCaminoLargo() {
        Grafo g = grafoChico();
        Planner planner(g);
        // 3000^2 * 2.5 = 2.25e7 > 2.0e7 (umbral) => no asequible.
        PlanCota plan = planner.planificar(Sonda{/*largoFinal*/ 3000, /*holgura*/ 0.3});

        assert(!contiene(plan, PasoCota::Heuristica::SCATTER));
        assert(contiene(plan, PasoCota::Heuristica::GRASP));
        cout << "test_scatterNoAsequibleCaminoLargo OK\n";
    }

    // GRASP siempre presente, con corte por meseta (paciencia > 0) y tope positivo.
    static void test_graspSiemprePresenteConMeseta() {
        Grafo g = grafoChico();
        Planner planner(g);
        PlanCota plan = planner.planificar(Sonda{3000, 0.3}); // sin Scatter

        const PasoCota* grasp = pasoDe(plan, PasoCota::Heuristica::GRASP);
        assert(grasp != nullptr);
        assert(grasp->iteraciones > 0);
        assert(grasp->paciencia > 0);
        assert(grasp->alpha >= 0.0 && grasp->alpha <= 1.0);
        assert(grasp->umbralRefine > 0);
        cout << "test_graspSiemprePresenteConMeseta OK\n";
    }

    // Regimen combinatorio (holgura alta = el peso no ata) => mas multi-start de
    // GRASP que en el regimen donde el peso ata.
    static void test_regimenCombinatorioSubeIteraciones() {
        Grafo g = grafoChico();
        Planner planner(g);

        PlanCota combinatorio = planner.planificar(Sonda{10, /*holgura*/ 0.9});
        PlanCota pesoAta      = planner.planificar(Sonda{10, /*holgura*/ 0.1});

        int itComb = pasoDe(combinatorio, PasoCota::Heuristica::GRASP)->iteraciones;
        int itAta  = pasoDe(pesoAta,      PasoCota::Heuristica::GRASP)->iteraciones;
        assert(itComb > itAta);
        cout << "test_regimenCombinatorioSubeIteraciones OK\n";
    }

    // El plan es determinista: mismo grafo + misma sonda => mismo plan.
    static void test_esDeterminista() {
        Grafo g = grafoChico();
        Planner planner(g);
        Sonda s{10, 0.3};
        PlanCota a = planner.planificar(s);
        PlanCota b = planner.planificar(s);

        assert(a.pasos.size() == b.pasos.size());
        for (size_t i = 0; i < a.pasos.size(); ++i)
            assert(a.pasos[i].heuristica == b.pasos[i].heuristica);
        cout << "test_esDeterminista OK\n";
    }
};

int main() {
    TestPlanner::test_scatterAsequibleCaminoCorto();
    TestPlanner::test_scatterNoAsequibleCaminoLargo();
    TestPlanner::test_graspSiemprePresenteConMeseta();
    TestPlanner::test_regimenCombinatorioSubeIteraciones();
    TestPlanner::test_esDeterminista();
    cout << "Todos los tests de Planner OK\n";
    return 0;
}
