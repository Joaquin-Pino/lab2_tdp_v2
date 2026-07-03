#include "../grafo/grafo.h"
#include "../camino/camino.h"
#include "../kopt/kopt.h"
#include <vector>

class Scatter {
private:
    const Grafo* grafo;

    Camino combinar(Camino c1, Camino c2);
    std::vector<Camino> generarSoluciones(int n);

public:

    Scatter();
    Scatter(const Grafo& grafo);

    Camino resolver();

};