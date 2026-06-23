#include "camino.h"
#include <stdexcept>

using namespace std;

Camino::Camino() : pesoTotal(0), beneficioTotal(0), grafo(nullptr) {}

Camino::Camino(vector<int> camino, const Grafo& grafo) :  pesoTotal(0), beneficioTotal(0), camino(camino){
    this->grafo = &grafo;
    calcularYAsignarPesoYBeneficio();
}

void Camino::agregarNodo(int id){
    if (visitados.count(id)) return;
    visitados.insert(id);

    if (!camino.empty()){
        int idUltimoNodo = camino.back();
        for (const Nodo& n : grafo->getVecinos(idUltimoNodo)){
            if (n.destino == id){
                pesoTotal += n.costo;
                beneficioTotal += n.beneficio;
                break;
            }
        }
    }

    camino.push_back(id);
}

void Camino::eliminarNodo(int id){
    // borrar del camino
    for (int i = 0; i < camino.size() ; i++){
        if (camino[i] == id){
            Nodo temp = grafo->getArista(camino[i - 1], id);
            Nodo temp2 = grafo->getArista(id, camino[i+1]);

            // reducir pesoTotal y beneficioTotal
            pesoTotal -= (temp.costo + temp2.costo);
            beneficioTotal -= (temp.beneficio + temp2.beneficio);

            camino.erase(camino.begin() + i); 
            break;
        }
    }
    // borrar de visitado
    visitados.erase(id);
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
    for (int i = 1; i < camino.size(); i++){
        Nodo temp = grafo->getArista(camino[i -1], camino[i]);
        costo += temp.costo;
        beneficio += temp.beneficio;
    }

    pesoTotal = costo;
    beneficioTotal = beneficio;
}

void Camino::marcarNodoVisitado(int id){
    if (visitados.find(id) != visitados.end()){
        return;
    }
    visitados.insert(id);
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

vector<int> Camino::getCamino() const{
    return camino;
}

int Camino::getBeneficioTotal(){
    return beneficioTotal;
}

int Camino::getPesoTotal(){
    return pesoTotal;
}

bool Camino::esCaminoCompleto(){
    if (camino.empty()) return false;
    return camino.front() == 0 && camino.back() == grafo->getCantVert() - 1;
}

float Camino::getRatioNodo(int id){
    //para cada nodo 
    for (int i = 1; i < (int)camino.size() - 1; i++){
        if (camino[i] == id){
            Nodo entrada = grafo->getArista(camino[i-1], id);
            Nodo salida  = grafo->getArista(id, camino[i+1]);
            int costo = entrada.costo + salida.costo;
            if (costo == 0) return 0.0f;
            return (float)(entrada.beneficio + salida.beneficio) / costo;
        }
    }
    return -1.0f;
}

void Camino::reemplazarNodo(int oldId, int newId, bool recalcular = true){
    for (int i = 1; i < (int)camino.size() - 1; i++){
        if (camino[i] == oldId){
            visitados.erase(oldId);
            camino[i] = newId;
            visitados.insert(newId);
            if (recalcular) calcularYAsignarPesoYBeneficio();
            return;
        }
    }
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