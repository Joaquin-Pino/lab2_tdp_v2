#pragma once

#include <random>

#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../solverGreedy/solverGreedy.h"

class Kopt {
private:
    const Grafo* grafo;
    
    bool verificarAristas(const std::vector<int>& combinacion, 
                        const std::vector<int>& candidatoCamino);

    std::mt19937* rng; // no-dueño: lo inyecta quien construye el solver


public:

    // rng compartido inyectado desde afuera (main/runner/test); debe sobrevivir
    // al Kopt, igual contrato que el puntero a Grafo.
    Kopt(const Grafo& grafo, std::mt19937& rng);
    // no es necesario el de copia, no hay asignacion dinaimica de memoria

    // K-OPT convencional: genera una solucion inicial con el goloso y la
    // mejora reordenando k nodos ya presentes en el camino (generalizacion
    // de 2-OPT). Busca hasta convergencia: termina cuando ninguna
    // combinacion/permutacion de k nodos mejora el beneficio.
    // primeraMejora = true (first-improvement, default): aplica el primer
    // candidato factible que mejore y reinicia la pasada.
    // primeraMejora = false (best-improvement / steepest descent): evalua
    // toda la vecindad de la pasada y aplica solo el mejor candidato.
    Camino resolver(bool primeraMejora = true, int k = 2);

    // Igual que resolver(), pero partiendo de un camino ya construido en
    // vez de llamar al goloso. Pensado para refinar el camino que entrega
    // granSalto() (o cualquier otro camino factible), por ejemplo dentro
    // de Breakout despues de un salto.
    // maxPasadas: tope de pasadas del bucle de mejora. -1 (default) = ilimitado
    // (corre hasta convergencia, como antes). Un valor >= 0 acota el costo en
    // caminos largos, donde cada pasada materializa C(L,2) combinaciones. Es
    // opt-in: los llamadores existentes (Breakout, 2-OPT) no lo pasan y no
    // cambian de comportamiento.
    Camino resolver(const Camino& inicial, bool primeraMejora = true, int k = 2,
                    int maxPasadas = -1);

    Camino granSalto(const Camino& inicial, int kSalto);

};