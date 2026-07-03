#pragma once

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../algoritmos/algoritmo.h"
#include "../solverGreedy/solverGreedy.h"

class Kopt {
private:
    const Grafo* grafo;
    int k;
    
    bool verificarAristas(const std::vector<int>& combinacion, 
                        const std::vector<int>& candidatoCamino);
    
public:

    Kopt();
    Kopt(const Grafo& grafo, int k);
    // no es necesario el de copia, no hay asignacion dinaimica de memoria

    // K-OPT convencional: genera una solucion inicial con el goloso y la
    // mejora reordenando k nodos ya presentes en el camino (generalizacion
    // de 2-OPT). Busca hasta convergencia: termina cuando ninguna
    // combinacion/permutacion de k nodos mejora el beneficio.
    // primeraMejora = true (first-improvement, default): aplica el primer
    // candidato factible que mejore y reinicia la pasada.
    // primeraMejora = false (best-improvement / steepest descent): evalua
    // toda la vecindad de la pasada y aplica solo el mejor candidato.
    Camino resolver(bool primeraMejora = true);

    // Igual que resolver(), pero partiendo de un camino ya construido en
    // vez de llamar al goloso. Pensado para refinar el camino que entrega
    // perturbar() (o cualquier otro camino factible), por ejemplo dentro
    // de Breakout despues de un salto.
    Camino resolver(const Camino& inicial, bool primeraMejora = true);

    // K-OPT con perturbacion: reemplaza los k nodos de peor razon
    // beneficio/costo del camino por k nodos no visitados del grafo.
    // A diferencia de resolver(), el resultado se acepta sin exigir
    // mejora del beneficio: es el "salto grande" que usa Breakout Search
    // para escapar de optimos locales, aceptando que empeore.
    Camino perturbar(const Camino& inicial);

};