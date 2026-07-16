#include "breakout.h"

using namespace std;

Breakout::Breakout(const Grafo& grafo, std::mt19937& rng, int maxIter, int L0): grafo(&grafo), rng(&rng), L0(L0), maxIteraciones(maxIter) {}

// Breakout Search: alterna saltos grandes (escapar del optimo local actual)
// con refinamiento 2-OPT (volver a caer en un optimo local), igual que
// simulated annealing pero con la magnitud del salto L como "temperatura":
// crece si la busqueda se estanca y se resetea apenas vuelve a mejorar.
Camino Breakout::resolver() {
    Kopt solverKopt(*grafo, *rng);

    Camino camino = generarSolucionInicial();
    Camino caminoBest = camino;

    int L = L0;
    int sinMejora = 0;
    int iteracion = 0;

    while (iteracion < maxIteraciones) {
        // salto grande: reordena L nodos al azar sin exigir mejora (Kopt::granSalto)
        Camino caminoSalto = solverKopt.granSalto(camino, L);
        // refinamiento: 2-OPT (k=2) steepest-descent hasta el siguiente optimo local
        Camino caminoRefinado = solverKopt.resolver(caminoSalto, false, 2);

        if (caminoRefinado.getBeneficioTotal() > caminoBest.getBeneficioTotal()) {
            caminoBest = caminoRefinado;
            sinMejora = 0;
            L = L0;                 // se resetea: la magnitud actual sí sirvió
        } else {
            sinMejora++;
            // Nota: la comparacion es contra maxIteraciones (tope global de
            // iteraciones), no contra un umbral de paciencia propio, asi que
            // en la practica L solo escala una vez, cerca del final de la
            // corrida (cuando sinMejora alcanza ese mismo tope).
            if (sinMejora >= maxIteraciones) {
                L++;                 // no ha mejorado en n iteraciones: perturbar más fuerte
                sinMejora = 0;
            }
        }

        // caminoRefinado se vuelve el punto de partida del proximo salto,
        // sea o no mejor que caminoBest (permite alejarse del optimo actual
        // en vez de quedar atrapado saltando siempre desde el mismo punto)
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