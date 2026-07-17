#pragma once

#include <vector>

#include "../grafo/grafo.h"

// Planner: mini-agente "plan-based" que decide que heuristica(s) usar como cota
// inferior del Branch and Bound. El motivo es que no se puede predecir la familia
// de grafos de evaluacion, asi que en vez de umbrales hardcodeados dentro del B&B
// (antes UMBRAL_SCATTER / ITER_GRASP_COTA) el Planner inspecciona el grafo
// (tamano, densidad, presupuesto) y DEVUELVE la configuracion; el B&B la ejecuta.
//
// El Planner es puro: solo depende de const Grafo& y no consume aleatoriedad. Eso
// lo vuelve trivial de testear (dado un grafo -> un plan) y separa la decision de
// la ejecucion. La invariante de aprobacion (B&B >= mejor cota) no depende de que
// plan se elija: el B&B corre todos los pasos y se queda con el mejor camino, con
// el Greedy como piso universal; el Planner solo afecta calidad de poda / tiempo.

// Un paso del plan: una heuristica a correr como candidata a cota, con los
// parametros que esa heuristica pueda necesitar. Los campos no usados por una
// heuristica se ignoran (structs de solo datos: no viola "sin funciones libres").
struct PasoCota {
    enum class Heuristica { GREEDY, KOPT, BREAKOUT, SCATTER, GRASP };

    Heuristica heuristica;
    int    iteraciones  = 0;    // maxIter para Scatter / Grasp / Breakout
    double alpha        = 0.3;  // Grasp: corte de la RCL
    int    tamPoblacion = 30;   // Scatter: tamano de poblacion inicial
    int    tamRefSet    = 10;   // Scatter: tamano del RefSet
    int    umbralRefine = 500;  // Grasp: sobre este N omite el 2-opt
};

// El plan es un portafolio de pasos. El B&B los corre todos y evaluarCandidato se
// queda con el de mayor beneficio. GREEDY va siempre primero como piso.
struct PlanCota {
    std::vector<PasoCota> pasos;
};

class Planner {
public:
    Planner(const Grafo& grafo);

    // Decide el plan a partir de las metricas del grafo. No ejecuta nada.
    PlanCota planificar() const;

private:
    const Grafo* grafo;

    // Frontera de tamano para el modo de trabajo. Bajo este N el grafo es "chico":
    // Scatter es barato y da la cota mas ajustada, asi que se corre en portafolio
    // con GRASP. Sobre este N Scatter se dispara (su 2-opt escala con el largo del
    // camino) y se usa solo GRASP escalado. Reproduce el viejo UMBRAL_SCATTER.
    static constexpr int UMBRAL_GRAFO_GRANDE = 1000;

    // Construye los pasos base de cada heuristica (params por defecto del PasoCota
    // salvo lo que se ajusta segun el grafo).
    PasoCota pasoGreedy() const;
    PasoCota pasoScatter(int iteraciones) const;
    PasoCota pasoGrasp(int iteraciones) const;

    // Iteraciones de GRASP escaladas por tamano: cuesta mas por construccion en
    // grafos grandes, asi que se baja el numero para respetar el tiempo de la cota.
    int iteracionesGrasp() const;
};
