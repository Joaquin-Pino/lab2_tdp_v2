#include "breakout.h"

using namespace std;

Breakout::Breakout(): grafo(nullptr), minimosLocales({}), L0(2), maxIteraciones(0) {}

Breakout::Breakout(const Grafo& grafo, int maxIter, int L0): grafo(&grafo), minimosLocales({}), L0(L0), maxIteraciones(maxIter) {}

Camino Breakout::resolver() {
    Kopt solverKopt(*grafo);

    Camino camino = generarSolucionInicial();
    Camino caminoBest = camino;

    int L = L0;
    int sinMejora = 0;
    int iteracion = 0;

    while (iteracion < maxIteraciones) {
        Camino caminoSalto = solverKopt.granSalto(camino, L);
        Camino caminoRefinado = solverKopt.resolver(caminoSalto, false, 2);

        vector<int> clave = caminoRefinado.getCamino();
        minimosLocales.insert(clave);

        if (caminoRefinado.getBeneficioTotal() > caminoBest.getBeneficioTotal()) {
            caminoBest = caminoRefinado;
            sinMejora = 0;
            L = L0;                 // se resetea: la magnitud actual sí sirvió
        } else {
            sinMejora++;
            if (sinMejora >= maxIteraciones) {
                L++;                 // no ha mejorado en n iteraciones: perturbar más fuerte
                sinMejora = 0;
            }
        }

        camino = caminoRefinado;
        iteracion++;
    }
    return caminoBest;
}

Camino Breakout::generarSolucionInicial(){
    SolverGreedy SolverGreedy(*grafo);
    // si el camino entregado por greedy no es completo (no llega al destino),
    Camino inicial = SolverGreedy.resolver();

    if (!inicial.llegaFinal()){
        int idFinal = inicial.getUltimoNodo();
        vector<int> temp = grafo->dijkstraCamino(idFinal, grafo->getIdNodoFinal());
        inicial.concatenar(temp);
    }

    return inicial;
}