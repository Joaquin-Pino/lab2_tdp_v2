#pragma once

#include <vector>

#include "../grafo/grafo.h"

// Planner: mini-agente "plan-based" que decide que heuristica(s) usar como cota
// inferior del Branch and Bound. El motivo es que no se puede predecir la familia
// de grafos de evaluacion, asi que en vez de umbrales hardcodeados dentro del B&B
// (antes UMBRAL_SCATTER / ITER_GRASP_COTA) el Planner decide en base a EVIDENCIA y
// DEVUELVE la configuracion; el B&B la ejecuta.
//
// El Planner es puro y determinista: no corre heuristicas ni consume rng. Recibe
// una Sonda (medidas de un par de construcciones que el B&B ya calculo: el largo
// real del camino y cuanto presupuesto deja libre el greedy) y a partir de ahi
// arma el plan. Contar nodos (el viejo N >= 1000) era un mal proxy: lo que dispara
// el costo de Scatter es el LARGO del camino final (su 2-opt escala ~O(L^2)), y ese
// largo no se puede estimar de las estadisticas del grafo porque los caminos usan
// las aristas mas baratas, no el costo medio. Por eso el B&B mide el largo real con
// una construccion y se lo pasa al Planner.
//
// La invariante de aprobacion (B&B >= mejor cota) no depende de que plan se elija:
// el B&B corre el greedy de piso y todos los pasos, y evaluarCandidato se queda con
// el mejor camino. El Planner solo afecta calidad de poda / tiempo, no la correccion.

// Medidas que el B&B toma antes de planificar y le entrega al Planner.
struct Sonda {
    // Largo (nro de nodos) del camino que arma una construccion GRASP. Es el
    // largo real que Scatter/2-opt recorreria: refleja el presupuesto llenado,
    // no el beeline corto del greedy.
    int largoCaminoFinal;
    // Fraccion del presupuesto que el camino greedy deja SIN usar (1 - pesoGreedy
    // / W). Alta => el peso "no ata" (regimen combinatorio, insertar nodos manda).
    double holguraGreedy;
};

// Un paso del plan: una heuristica a correr como candidata a cota, con los
// parametros que esa heuristica pueda necesitar. Los campos no usados por una
// heuristica se ignoran (structs de solo datos: no viola "sin funciones libres").
struct PasoCota {
    enum class Heuristica { GREEDY, KOPT, BREAKOUT, SCATTER, GRASP };

    Heuristica heuristica;
    int    iteraciones  = 0;    // tope de iteraciones (Scatter / Grasp / Breakout)
    int    paciencia    = 0;    // Grasp: corta por meseta tras N iters sin mejora
    double alpha        = 0.3;  // Grasp: corte de la RCL
    int    tamPoblacion = 30;   // Scatter: tamano de poblacion inicial
    int    tamRefSet    = 10;   // Scatter: tamano del RefSet
    int    umbralRefine = 500;  // Grasp: sobre este N omite el 2-opt
};

// El plan es un portafolio de pasos. El B&B corre el greedy de piso por su cuenta
// y luego cada paso; evaluarCandidato se queda con el de mayor beneficio.
struct PlanCota {
    std::vector<PasoCota> pasos;
};

class Planner {
public:
    Planner(const Grafo& grafo);

    // Decide el plan a partir de las metricas del grafo y de la sonda. No ejecuta.
    PlanCota planificar(const Sonda& sonda) const;

private:
    const Grafo* grafo;

    // Asequibilidad de Scatter: su 2-opt escala ~O(L^2) por camino y toca el grado
    // medio, asi que el costo por iteracion ~ L^2 * gradoMedio. Sobre este umbral
    // Scatter se vuelve demasiado caro para la cota y se usa solo GRASP. Calibrado
    // sobre los grafos de ejemplo: deja adentro grafoGrande/mil3/gigante (Scatter
    // corre en seg.) y afuera mil/mil2/ohMypc (donde tardaba decenas de segundos).
    static constexpr double UMBRAL_COSTO_SCATTER = 2.0e7;

    // Sobre esta holgura del greedy se considera que el peso "no ata" (regimen
    // combinatorio): conviene mas multi-start de GRASP para que rellenar() inserte
    // nodos de alto beneficio.
    static constexpr double HOLGURA_COMBINATORIA = 0.5;

    // Topes de GRASP y paciencia del corte por meseta. El tope alto solo acota el
    // peor caso; en la practica corta la meseta.
    static constexpr int ITER_GRASP_BASE          = 20;
    static constexpr int ITER_GRASP_COMBINATORIO  = 40;
    static constexpr int PACIENCIA_GRASP          = 5;

    // Grado medio del grafo no dirigido: getDensidad()*(N-1) = 2M/N.
    double gradoMedio() const;

    PasoCota pasoScatter(int iteraciones) const;
    PasoCota pasoGrasp(int iteraciones, int paciencia) const;
};
