#include "scatter.h"
#include <algorithm>
using namespace std;

Scatter::Scatter(): grafo(nullptr){}

Scatter::Scatter(const Grafo& grafo): grafo(&grafo) {}

Camino Scatter::resolver(){
    Kopt koptSolver(*grafo);
    vector<Camino> soluciones = generarSoluciones(100);

    for (Camino& c : soluciones){
        c = koptSolver.resolver(c, false, 2); // optimizamos con 2-opt
    }

    // ordenamos por beneficio del camino
    sort(soluciones.begin(), soluciones.end(), greater<Camino>());

    // agregamos los 10-20 caminos  al refSEt

    // iter = 0
    // minentas iter < maxIter
        // seleccionar pares de caminos aun no combinaod 

        // si pares esta vacio
            // mejor <- mejor solucion de refset
            // RefSet ← {mejor} ∪ generarSoluciones(9)
            // RefSet ← mejorar las 9 nuevas con Kopt.resolver
        // si no:
            // set temporal para guardar combinaciones
            // para cada par (c1, c2):
                // comino_nuevo = combinar(c1,c2)
                // comino_nuevo = kopt(comino_nuevo, 2) // refinamos por 2-opt
                // marcamos (c1, c2) como visto
            
            // agregamos el set temporal al refset

        // iter++

    // retornar mejor solucion del refset
            
}