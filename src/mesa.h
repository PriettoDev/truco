#ifndef MESA_H
#define MESA_H

#include "jogador.h"

#define MAX_JOGADOR 5   // 4 jogadores + 1 posicao sempre vazia (fila circular)
#define NUM_JOGADORES 4  // jogadores de verdade na mesa
#define PONTOS_PARA_VENCER 12

// Fila circular que guarda a ordem de jogo: os indices, na ordem em que jogam
typedef struct {
    Jogador *posicoes[MAX_JOGADOR];
    int ini, fim;
} Ordem;

// Representa a mesa: os 4 jogadores sentados e a ordem de jogo atual
typedef struct {
    Jogador jogadores[NUM_JOGADORES];
    Ordem ordem;
    Carta vira; //Mesa guarda a carta virada da rodada
    int numeroDaRodada;
    int inicioDaRodada;   // assento de quem abriu a rodada atual (-1 = ainda nao definido)
    int pontosDupla1;
    int pontosDupla2;
} Mesa;

void inicializarMesa(Ordem *o);
int mesaVazia(Ordem *o);
int mesaCheia(Ordem *o);
int inserirMesa(Ordem *o, Jogador *jogador);
int removerMesa(Ordem *o, Jogador **jogador);
int tamanhoMesa(Ordem *o);
void girarMesa(Ordem *o);
Jogador *frenteMesa(Ordem *o);
int definirPrimeiroDaMao(Mesa *mesa, int assento);
int iniciarNovaRodada(Mesa *mesa);
void imprimirOrdemDaMao(const Mesa *mesa);

#endif