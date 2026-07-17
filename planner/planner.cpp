#include "planner.h"

Planner::Planner(const Grafo& grafo) : grafo(&grafo) {}

PasoCota Planner::pasoGreedy() const {
    PasoCota paso;
    paso.heuristica = PasoCota::Heuristica::GREEDY;
    return paso;
}

PasoCota Planner::pasoScatter(int iteraciones) const {
    PasoCota paso;
    paso.heuristica = PasoCota::Heuristica::SCATTER;
    paso.iteraciones = iteraciones;
    // tamPoblacion / tamRefSet se dejan en el default calibrado (30 / 10).
    return paso;
}

PasoCota Planner::pasoGrasp(int iteraciones) const {
    PasoCota paso;
    paso.heuristica = PasoCota::Heuristica::GRASP;
    paso.iteraciones = iteraciones;
    return paso;
}

int Planner::iteracionesGrasp() const {
    int n = grafo->getCantVert();
    // Escala inversa al tamano: en grafos chicos GRASP es barato y conviene
    // exprimirlo; en grafos grandes cada construccion cuesta, asi que se baja el
    // numero para que la cota no domine el tiempo total del B&B. El punto de
    // partida (20) reproduce el viejo ITER_GRASP_COTA.
    if (n <= UMBRAL_GRAFO_GRANDE) return 20;
    if (n <= 5000) return 12;
    return 6;
}

PlanCota Planner::planificar() const {
    PlanCota plan;

    // Piso universal: barato en cualquier tamano, garantiza al menos un candidato.
    plan.pasos.push_back(pasoGreedy());

    int n = grafo->getCantVert();

    if (n <= UMBRAL_GRAFO_GRANDE) {
        // Grafo chico: portafolio Scatter + GRASP. Scatter da la cota mas ajustada
        // (combina soluciones) y a este tamano es barato; sumar GRASP cubre los
        // casos donde el operador de Scatter no encaja (mas robusto ante grafos
        // desconocidos, que es justo el motivo del Planner).
        plan.pasos.push_back(pasoScatter(5));
        plan.pasos.push_back(pasoGrasp(iteracionesGrasp()));
    } else {
        // Grafo grande: Scatter se dispara por los caminos largos, se usa solo
        // GRASP escalado, que llena el presupuesto casi tan bien a una fraccion
        // del costo.
        plan.pasos.push_back(pasoGrasp(iteracionesGrasp()));
    }

    return plan;
}
