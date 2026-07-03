#include "../kopt/kopt.h"
#include "../grafo/grafo.h"
#include "../camino/camino.h"

#include <unordered_set>

class Breakout {
private:
    const Grafo* grafo;
    std::unordered_set<Camino> minimosLocales;
    int L; //magintud del salto inicial (k para el kopt)
    

public:
    Breakout();
    Breakout(const Grafo& grafo);

    Camino resolver();

};