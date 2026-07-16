#include "camino.h"
#include <stdexcept>

using namespace std;

Camino::Camino(vector<int> camino, const Grafo& grafo) :  pesoTotal(0), beneficioTotal(0), camino(camino){
    this->grafo = &grafo;
    for (int id : camino) visitados.insert(id);
    calcularYAsignarPesoYBeneficio();
}

bool Camino::agregarNodo(int id){
    // TODO: si id ya esta visitado se descarta en silencio (sin sumar peso ni
    // agregar al vector). Esto es intencional para concatenar(), pero si el
    // camino de completacion (dijkstraCamino) pasa por un nodo ya presente en
    // el prefijo, quedan dos nodos consecutivos que quizas no tienen arista y
    // el peso/beneficio del tramo se calcula mal. Considerar retornar bool para
    // avisar cuando se descarto un nodo y validar factibilidad en concatenar().
    if (visitados.count(id) > 0) return false;
    visitados.insert(id);

    if (!camino.empty()){
        int idUltimoNodo = camino.back();
        if (grafo->existeArista(idUltimoNodo, id)) {
            pesoTotal      += grafo->getPeso(idUltimoNodo, id);
            beneficioTotal += grafo->getBeneficio(idUltimoNodo, id);
        }
    }

    camino.push_back(id);
    return true;
}

bool Camino::eliminarNodo(int id){
    int i = getPosicionNodo(id);

    // Solo nodos interiores: el primero y el ultimo son los extremos que
    // definen el camino, y ademas no tienen una arista de cada lado.
    if (i <= 0 || i >= (int)camino.size() - 1) return false;

    int prev = camino[i - 1];
    int sig  = camino[i + 1];

    // Al sacar id, prev y sig quedan consecutivos: si esa arista no existe el
    // camino resultante no seria recorrible en el grafo.
    if (!grafo->existeArista(prev, sig)) return false;

    Nodo entrada = grafo->getArista(prev, id);
    Nodo salida  = grafo->getArista(id, sig);
    Nodo puente  = grafo->getArista(prev, sig);

    // Se reemplazan las dos aristas del nodo por la arista puente.
    pesoTotal      += puente.costo     - entrada.costo     - salida.costo;
    beneficioTotal += puente.beneficio - entrada.beneficio - salida.beneficio;

    camino.erase(camino.begin() + i);
    visitados.erase(id);
    return true;
}

// TODO: que esto se encargue solo de intercambiar
bool Camino::intercambiarNodos(int id1, int id2){
    int indx1 = -1;
    int indx2 = -1;

    for (int i = 0; i < (int)camino.size(); i++){
        if (id1 == camino[i]) indx1 = i;
        if (id2 == camino[i]) indx2 = i;
    }

    if (indx1 == -1 || indx2 == -1) return false;

    swap(camino[indx1], camino[indx2]);
    calcularYAsignarPesoYBeneficio();
    return true;
}

void Camino::calcularYAsignarPesoYBeneficio(){
    if (camino.size() == 0){
        return;
    }

    int costo = 0;
    int beneficio = 0;
    for (int i = 1; i < (int) camino.size(); i++){
        Nodo temp = grafo->getArista(camino[i -1], camino[i]);
        costo += temp.costo;
        beneficio += temp.beneficio;
    }

    pesoTotal = costo;
    beneficioTotal = beneficio;
}

bool Camino::nodoFueVisitado(int id) const {
    return visitados.find(id) != visitados.end();
}

bool Camino::verificarCamino(int wMax){
    return pesoTotal <= wMax;
}

int Camino::getUltimoNodo(){
    if (camino.size() == 0){
        throw runtime_error("no se puede obtener el ultimo nodo de un camino vacio, error en metodo getUltimoNodo");
    }

    return camino[camino.size() -1];
}

const vector<int>& Camino::getCamino() const{
    return camino;
}

int Camino::getBeneficioTotal() const {
    return beneficioTotal;
}

int Camino::getPesoTotal(){
    return pesoTotal;
}

bool Camino::esCaminoCompleto(){
    if (camino.empty()) return false;
    return camino.front() == 0 && camino.back() == grafo->getCantVert() - 1;
}

int Camino::getLargo(){
    return camino.size();
}

int Camino::getPosicionNodo(int idNodo){
    for (int i = 0; i < (int) camino.size(); i++){
        if (camino[i] == idNodo){
            return i;
        }
    }

    return -1;
}

bool Camino::llegaFinal(){
    if (camino.back() == grafo->getIdNodoFinal()) {
        return true;
    }
    return false;
}


void Camino::concatenar(const vector<int>& c) {
    for (int id : c){
        if (nodoFueVisitado(id)){
            // truncar el prefijo hasta ese nodo para mantener el camino simple.
            // (agregarNodo dejaria un hueco sin arista -> revienta getPeso en 2-OPT)
            while (!camino.empty() && camino.back() != id) eliminarUltimo();
        } else {
            agregarNodo(id); // arista prev->id garantizada por dijkstraCamino
        }
    }
}

void Camino::eliminarUltimo(){
    if (camino.empty()) return;
    int ultimo = camino.back();
    if (camino.size() >= 2){
        Nodo a = grafo->getArista(camino[camino.size() - 2], ultimo);
        pesoTotal -= a.costo;
        beneficioTotal -= a.beneficio;
    }
    visitados.erase(ultimo);
    camino.pop_back();
}