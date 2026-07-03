#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <limits>

#include "grafo/grafo.h"
#include "camino/camino.h"
#include "solverGreedy/solverGreedy.h"
#include "kopt/kopt.h"
#include "breakout/breakout.h"

using namespace std;
using namespace std::chrono;

class MenuOrienteering {
private:
    unique_ptr<Grafo> grafo;

    bool hayGrafoCargado() const {
        if (!grafo) {
            cout << "No hay un grafo cargado. Use la opcion 1 primero.\n";
            return false;
        }
        return true;
    }

    static void limpiarLineaPendiente() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    static void imprimirResultado(const string& nombre, Camino& camino, double ms) {
        vector<int> c = camino.getCamino();
        cout << "\n--- " << nombre << " ---\n";
        cout << "Camino: ";
        for (size_t i = 0; i < c.size(); i++) {
            cout << c[i];
            if (i + 1 < c.size()) cout << " -> ";
        }
        cout << "\n";
        cout << "Peso total: " << camino.getPesoTotal() << "\n";
        cout << "Beneficio total: " << camino.getBeneficioTotal() << "\n";
        cout << "Tiempo de ejecucion: " << ms << " ms\n";
    }

    void cargarGrafo() {
        cout << "Ingrese el nombre del archivo: ";
        string filename;
        cin >> filename;
        try {
            grafo = make_unique<Grafo>(Grafo::cargarDesdeArchivo(filename));
            cout << "Grafo cargado correctamente (" << grafo->getCantVert() << " nodos, maxW=" << grafo->getMaxW() << ").\n";
        } catch (const exception& e) {
            cout << "Error al cargar el grafo: " << e.what() << "\n";
        }
    }

    void ejecutarMejorAlgoritmo() {
        if (!hayGrafoCargado()) return;
        cout << "Branch and Bound aun no esta implementado. Pruebe la opcion 3 con los algoritmos disponibles.\n";
    }

    void ejecutar2Opt() {
        Kopt kopt(*grafo);
        auto inicio = high_resolution_clock::now();
        Camino solucion = kopt.resolver();
        auto fin = high_resolution_clock::now();
        double ms = duration<double, milli>(fin - inicio).count();
        imprimirResultado("2-OPT", solucion, ms);
    }

    void ejecutarBreakout() {
        const int maxIter = 100;
        const int L0 = 2;
        Breakout breakout(*grafo, maxIter, L0);
        auto inicio = high_resolution_clock::now();
        Camino solucion = breakout.resolver();
        auto fin = high_resolution_clock::now();
        double ms = duration<double, milli>(fin - inicio).count();
        imprimirResultado("Breakout", solucion, ms);
    }

    void seleccionarAlgoritmo() {
        if (!hayGrafoCargado()) return;

        cout << "\n1. 2-OPT\n2. Breakout\n3. Scatter\n4. Branch and Bound\nOpcion: ";
        int opcion;
        cin >> opcion;
        if (cin.fail()) {
            cin.clear();
            limpiarLineaPendiente();
            cout << "Opcion invalida.\n";
            return;
        }

        switch (opcion) {
            case 1:
                ejecutar2Opt();
                break;
            case 2:
                ejecutarBreakout();
                break;
            case 3:
                cout << "Scatter aun no esta implementado.\n";
                break;
            case 4:
                cout << "Branch and Bound aun no esta implementado.\n";
                break;
            default:
                cout << "Opcion invalida.\n";
        }
    }

    void ingresarCaminoManual() {
        if (!hayGrafoCargado()) return;

        limpiarLineaPendiente();
        cout << "Ingrese la secuencia de nodos separados por espacio: ";
        string linea;
        getline(cin, linea);

        istringstream iss(linea);
        vector<int> nodos;
        int id;
        while (iss >> id) nodos.push_back(id);

        if (nodos.empty()) {
            cout << "No se ingreso ningun nodo.\n";
            return;
        }

        try {
            Camino camino(nodos, *grafo);
            cout << "Peso total: " << camino.getPesoTotal() << "\n";
            cout << "Beneficio total: " << camino.getBeneficioTotal() << "\n";
        } catch (const exception& e) {
            cout << "Camino invalido: " << e.what() << "\n";
        }
    }

public:
    void ejecutar() {
        bool salir = false;
        while (!salir) {
            cout << "\n===== Orienteering Problem =====\n";
            cout << "1. Cargar archivo de grafo\n";
            cout << "2. Ejecutar mejor algoritmo (Branch and Bound)\n";
            cout << "3. Seleccionar algoritmo especifico\n";
            cout << "4. Ingresar camino manualmente\n";
            cout << "5. Salir\n";
            cout << "Opcion: ";

            int opcion;
            cin >> opcion;
            if (cin.fail()) {
                cin.clear();
                limpiarLineaPendiente();
                cout << "Opcion invalida.\n";
                continue;
            }

            switch (opcion) {
                case 1:
                    cargarGrafo();
                    break;
                case 2:
                    ejecutarMejorAlgoritmo();
                    break;
                case 3:
                    seleccionarAlgoritmo();
                    break;
                case 4:
                    ingresarCaminoManual();
                    break;
                case 5:
                    salir = true;
                    break;
                default:
                    cout << "Opcion invalida.\n";
            }
        }
    }
};

int main() {
    MenuOrienteering menu;
    menu.ejecutar();
    return 0;
}
