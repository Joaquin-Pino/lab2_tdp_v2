#pragma once
#include <vector>

// Utilidades combinatorias puras (sin estado, sin dependencia de Grafo/Camino)
// usadas por Kopt para generar la vecindad de un movimiento K-OPT: que k
// posiciones del camino tocar (combinar) y en que orden probar esos k nodos
// (permutar).
class Algoritmo{
private:
    // Backtracking incluir/excluir sobre candidatos[i..]: por cada indice i se
    // exploran las dos ramas (agregarlo a acum y avanzar, o saltarlo y avanzar)
    // sin nunca retroceder antes de i, por eso nunca aparecen (a,b) y (b,a) a
    // la vez. Corta la rama en cuanto acum llega a tamano k.
    static void combinarAux(const std::vector<int>& candidatos, int k, int i, std::vector<int>& acum, std::vector<std::vector<int>>& resultado);
    // Backtracking clasico con arreglo de usados: en cada nivel prueba TODOS
    // los candidatos no usados (no solo desde el ultimo indice, a diferencia
    // de combinarAux), por lo que (a,b) y (b,a) si aparecen ambos. usados[]
    // se marca antes de bajar un nivel y se desmarca al volver (mismo slot se
    // reutiliza en cada llamada, no se copia).
    static void permutarAux(const std::vector<int>& candidatos, int k, std::vector<bool>& usados, std::vector<int>& acum, std::vector<std::vector<int>>& resultado);

public:
    // Todos los subconjuntos de tamano k de candidatos, sin importar el orden
    // (C(n,k) resultados). Vacio si k < 0 o k > candidatos.size().
    static std::vector<std::vector<int>> combinar(const std::vector<int>& candidatos, int k);

    // Todas las k-tuplas ordenadas de elementos distintos de candidatos
    // (P(n,k) = n!/(n-k)! resultados). k=0 devuelve un unico resultado vacio.
    // Vacio si k < 0 o k > candidatos.size().
    static std::vector<std::vector<int>> permutar(const std::vector<int>& candidatos, int k);
};