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
    if ((int) acum.size() == k){
        resultado.push_back(acum);
        return;
    }

    if (i >= (int)candidatos.size()) {
        return;
    }

    acum.push_back(candidatos[i]);
    combinarAux(candidatos, k, i+1, acum, resultado);
    acum.pop_back();

    //no elegimos el elemento
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

    if((int) acum.size() == k){
        resultado.push_back(acum);
        return;
    }

    for (int i = 0; i < (int)candidatos.size(); ++i) {

        // Control básico de permutación: no usar el mismo elemento dos veces
        if (usados[i]) continue;

        acum.push_back(candidatos[i]);
        usados[i] = true;

        permutarAux(candidatos, k, usados, acum, resultado);

        usados[i] = false;
        acum.pop_back();
    }
}
