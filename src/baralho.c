#include <stdio.h>
#include "baralho.h"

void inicializaBaralho(Baralho *b) {
    b->topo = -1;
}

void criaBaralho(Baralho *b) {
    int posicao = 0;

    for (int n = OUROS; n <= PAUS; n++) {
        for (int v = QUATRO; v <= TRES; v++) {

            b->cartas[posicao].naipe = n;
            b->cartas[posicao].valor = v;

            posicao++;
        }
    }

    b->topo = MAX_CARTAS - 1;
}

int baralhoVazio(Baralho *b) {
    if (b->topo == -1)
        return 1;

    return 0;
}

int retiraCarta(Baralho *b, Carta *c) {
    if (baralhoVazio(b))
        return 0;

    *c = b->cartas[b->topo];
    b->topo--;

    return 1;
}

void imprimeBaralho(Baralho b) {
    for (int i = b.topo; i >= 0; i--) {
        carta_imprimir(b.cartas[i]);
        printf("\n");
    }
}
