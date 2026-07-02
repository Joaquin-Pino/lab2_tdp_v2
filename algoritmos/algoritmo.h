#pragma once
#include <vector>

class Algoritmo{
private:
    static void combinarAux(const std::vector<int>& candidatos, int k, int i, std::vector<int>& acum, std::vector<std::vector<int>>& resultado);
    static void permutarAux(const std::vector<int>& candidatos, int k, std::vector<bool>& usados, std::vector<int>& acum, std::vector<std::vector<int>>& resultado);

public:
    static std::vector<std::vector<int>> combinar(const std::vector<int>& candidatos, int k);

    static std::vector<std::vector<int>> permutar(const std::vector<int>& candidatos, int k);
};