#pragma once

// Una arista vista desde uno de sus extremos: "desde aca, hacia destino, a
// este costo/beneficio". Es el elemento de Grafo::listaAdy[origen]; el
// extremo 'origen' es implicito (el indice de la lista), no se guarda aca.
struct Nodo {
    int destino;
    int costo;
    int beneficio;
};