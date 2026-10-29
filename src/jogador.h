#ifndef JOGADOR_H 
#define JOGADOR_H
#include "carta.h"

typedef struct {
    char nome [30];
    Carta mao [3];
    int qtdCartas;
    int dupla;
} Jogador;


Jogador jogador_criar(char nome [], int dupla);
void jogador_distribuir_cartas(Jogador *j, Carta c1, Carta c2, Carta c3);
void jogador_imprimir(Jogador *j);
int jogador_jogar_carta(Jogador *j, int pos, Carta *c);

#endif