#pragma once

#include "../kopt/kopt.h"
#include "../grafo/grafo.h"
#include "../camino/camino.h"

#include <random>

class Breakout {
private:
    const Grafo* grafo;
    std::mt19937* rng; // no-dueño: lo inyecta quien construye el solver; se propaga al Kopt
    int L0; //magintud del salto inicial (k para el kopt)
    int maxIteraciones; // max iitreaciones sin mejora

    Camino generarSolucionInicial();
public:
    // rng compartido inyectado desde afuera; debe sobrevivir al Breakout.
    Breakout(const Grafo& grafo, std::mt19937& rng, int maxIter, int L0 = 2);

    Camino resolver();

};