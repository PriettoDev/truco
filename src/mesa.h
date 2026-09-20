#ifndef MESA_H
#define MESA_H

#include "jogador.h"

#define MAX_JOGADOR 5   // 4 jogadores + 1 posicao sempre vazia (fila circular)

// Fila circular que guarda a ordem de jogo: os indices, na ordem em que jogam
typedef struct {
    int posicoes[MAX_JOGADOR];
    int ini, fim;
} Ordem;

// Representa a mesa: os 4 jogadores sentados e a ordem de jogo atual
typedef struct {
    Jogador jogadores[4];
    Ordem ordem;
} Mesa;

void inicializarMesa(Ordem *o);
int mesaVazia(Ordem *o);
int mesaCheia(Ordem *o);
int inserirMesa(Ordem *o, int indiceJogador);
int removerMesa(Ordem *o, int *indiceJogador);
int tamanhoMesa(Ordem *o);
void girarMesa(Ordem *o);
void imprimirOrdemDaMao(Mesa mesa);

#endif