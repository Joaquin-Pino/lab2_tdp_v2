#pragma once

#include "../kopt/kopt.h"
#include "../grafo/grafo.h"
#include "../camino/camino.h"

#include <unordered_set>
#include <random>

struct VectorIntHash {
    size_t operator()(const std::vector<int>& v) const {
        size_t seed = v.size();
        for (int x : v) {
            seed ^= std::hash<int>{}(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

class Breakout {
private:
    const Grafo* grafo;
    std::mt19937* rng; // no-dueño: lo inyecta quien construye el solver; se propaga al Kopt
    std::unordered_set<std::vector<int>, VectorIntHash> minimosLocales;
    int L0; //magintud del salto inicial (k para el kopt)
    int maxIteraciones; // max iitreaciones sin mejora


    bool hayMejora(Camino anterior, Camino nuevo);
    Camino generarSolucionInicial();
public:
    Breakout();
    // rng compartido inyectado desde afuera; debe sobrevivir al Breakout.
    Breakout(const Grafo& grafo, std::mt19937& rng, int maxIter, int L0 = 2);

    Camino resolver();

};