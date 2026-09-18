#ifndef BARALHO_H
#define BARALHO_H

#include "carta.h"

#define MAX_CARTAS 40

typedef struct {
    Carta cartas[MAX_CARTAS];
    int topo;
} Baralho;

void inicializaBaralho(Baralho *b);
void criaBaralho(Baralho *b);
int baralhoVazio(Baralho *b);
int retiraCarta(Baralho *b, Carta *c);
void imprimeBaralho(Baralho b);

#endif
