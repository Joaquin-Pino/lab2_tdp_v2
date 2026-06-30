#pragma once
#include <vector>
#include "../grafo/grafo.h"

class Algoritmo{
private:
    static void combinarAux(const std::vector<int>& candidatos, int k, int i, std::vector<int>& acum, std::vector<std::vector<int>>& resultado);
    static void permutarAux(const std::vector<int>& candidatos, const std::vector<std::pair<int,int>>& huecos, int k, int j, std::vector<bool>& usados, std::vector<int>& acum, std::vector<std::vector<int>>& resultado, const Grafo& grafo);

public:
    static std::vector<std::vector<int>> combinar(const std::vector<int>& candidatos, int k);

    static std::vector<std::vector<int>> permutar(const std::vector<int>& candidatos, const std::vector<std::pair<int,int>>& huecos, int k, const Grafo& grafo);
};