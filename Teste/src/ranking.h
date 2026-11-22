#ifndef RANKING_H
#define RANKING_H

#include "jogador.h"

// nao incluir historico.h aqui (conflita com a arvore.h)

#define RANKING_NUM_JOGADORES 4

typedef struct {
    int maosGanhas[RANKING_NUM_JOGADORES];   // indice = assento
} Ranking;

void ranking_iniciar(Ranking *r);
void ranking_registrar_mao(Ranking *r, int duplaVencedora);
void ranking_exibir_mural(const Ranking *r, const Jogador jogadores[4]);

#endif