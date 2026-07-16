#include "algoritmo.h"

using namespace std;

vector<vector<int>> Algoritmo::combinar(const vector<int>& candidatos, int k) {
    vector<vector<int>> res;
    vector<int> acum;
    if (k > (int)candidatos.size() || k < 0) return res;
    combinarAux(candidatos, k, 0, acum, res);

    return res;
}
void Algoritmo::combinarAux(const vector<int>& candidatos, int k, int i, vector<int>& acum, vector<vector<int>>& resultado){
    // caso base 1: acum ya tiene k elementos -> es una combinacion completa
    if ((int) acum.size() == k){
        resultado.push_back(acum);
        return;
    }

    // caso base 2: no quedan mas candidatos por considerar y aun falta
    // completar acum -> esta rama no llega a tamano k, se descarta
    if (i >= (int)candidatos.size()) {
        return;
    }

    // rama "incluir": candidatos[i] entra en la combinacion
    acum.push_back(candidatos[i]);
    combinarAux(candidatos, k, i+1, acum, resultado);
    acum.pop_back(); // deshacer antes de probar la otra rama (backtrack)

    // rama "excluir": candidatos[i] queda afuera, se sigue igual desde i+1.
    // Como ambas ramas avanzan a i+1 (nunca se vuelve a un indice anterior),
    // el orden relativo de candidatos se conserva y cada subconjunto se
    // genera una unica vez (sin permutaciones del mismo conjunto).
    combinarAux(candidatos, k, i + 1, acum, resultado);
}

vector<vector<int>> Algoritmo::permutar(const vector<int>& candidatos, int k) {
    vector<vector<int>> res;
    vector<int> acum;
    vector<bool> usados(candidatos.size(), false);
    if (k > (int)candidatos.size() || k < 0) return res;

    permutarAux(candidatos, k, usados, acum, res);

    return res;
}

void Algoritmo::permutarAux(const vector<int>& candidatos, int k, vector<bool>& usados,
                vector<int>& acum, vector<vector<int>>& resultado){

    // caso base: acum ya tiene k elementos -> es una k-tupla completa.
    // (si k==0 esto es cierto de entrada y agrega una unica tupla vacia)
    if((int) acum.size() == k){
        resultado.push_back(acum);
        return;
    }

    // A diferencia de combinarAux, aca se reinicia el for desde 0 en cada
    // nivel de recursion (no desde un indice creciente): por eso, para un
    // mismo conjunto de k elementos, se generan sus k! ordenes distintos.
    for (int i = 0; i < (int)candidatos.size(); ++i) {

        // Control básico de permutación: no usar el mismo elemento dos veces
        if (usados[i]) continue;

        acum.push_back(candidatos[i]);
        usados[i] = true;

        permutarAux(candidatos, k, usados, acum, resultado);

        // deshacer antes de probar el siguiente i (backtrack): usados y acum
        // vuelven a como estaban al entrar a esta llamada
        usados[i] = false;
        acum.pop_back();
    }
}
