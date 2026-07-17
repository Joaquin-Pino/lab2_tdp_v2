#include "planner.h"

Planner::Planner(const Grafo& grafo) : grafo(&grafo) {}

double Planner::gradoMedio() const {
    int n = grafo->getCantVert();
    if (n <= 1) return 0.0;
    return grafo->getDensidad() * (n - 1); // 2M/(N(N-1)) * (N-1) = 2M/N
}

PasoCota Planner::pasoScatter(int iteraciones) const {
    PasoCota paso;
    paso.heuristica = PasoCota::Heuristica::SCATTER;
    paso.iteraciones = iteraciones;
    // tamPoblacion / tamRefSet en el default calibrado (30 / 10).
    return paso;
}

PasoCota Planner::pasoGrasp(int iteraciones, int paciencia) const {
    PasoCota paso;
    paso.heuristica = PasoCota::Heuristica::GRASP;
    paso.iteraciones = iteraciones;
    paso.paciencia = paciencia;
    return paso;
}

PlanCota Planner::planificar(const Sonda& sonda) const {
    PlanCota plan;

    int n = grafo->getCantVert();
    int Lf = sonda.largoCaminoFinal;

    // Costo estimado de una iteracion de Scatter en ESTE grafo, a partir del largo
    // real del camino (medido por el B&B) y el grado medio. Si la sonda no logro
    // construir un camino (Lf <= 1) se cae a un criterio conservador por tamano.
    bool scatterAsequible;
    if (Lf <= 1) {
        scatterAsequible = (n <= 300);
    } else {
        double costoScatter = (double)Lf * Lf * gradoMedio();
        scatterAsequible = costoScatter <= UMBRAL_COSTO_SCATTER;
    }

    // Scatter da la cota mas ajustada (combina soluciones) pero solo se paga cuando
    // es asequible; en grafos con caminos largos se dispara y se omite.
    if (scatterAsequible)
        plan.pasos.push_back(pasoScatter(5));

    // GRASP siempre: es barato y su rellenar() cubre el regimen combinatorio (donde
    // el peso no ata y lo que sube el beneficio es insertar nodos). Mas multi-start
    // cuando el greedy dejo mucho presupuesto libre; el corte por meseta evita
    // gastar iteraciones de mas.
    int tope = (sonda.holguraGreedy > HOLGURA_COMBINATORIA)
                   ? ITER_GRASP_COMBINATORIO
                   : ITER_GRASP_BASE;
    plan.pasos.push_back(pasoGrasp(tope, PACIENCIA_GRASP));

    return plan;
}
