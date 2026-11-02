#ifndef RODADA_H
#define RODADA_H

#include "carta.h"

#define TOTAL_CARTAS  4    /* uma carta por jogador em cada rodada */
#define EMPATE         -1    /* rodada empatada entre as duplas */
#define INDEFINIDO     -2    /* rodada ainda nao apurada */

typedef struct {
    Carta carta;
    int   jogador;   /* assento das duplas na mesa: 0/2 = Dupla 1 (0) e 1/3 = Dupla 2 (1) */
    int   dupla;     /* 0 ou 1 -  Dupla 1 (0)/ Dupla 2 (1) */
} Jogada;

typedef struct {
    Jogada jogadas[TOTAL_CARTAS];
    int    qtd;               /* quantas cartas ja estao na mesa */
    int    dupla_vencedora;   /* 0, 1, EMPATE ou INDEFINIDO */
    int    jogador_vencedor;  /* quem fez a rodada; -1 se empatou */
} Rodada;

void rodada_iniciar(Rodada *r);
int  rodada_jogar(Rodada *r, Carta c, int jogador, int dupla);
int  rodada_completa(const Rodada *r);
void rodada_apurar(Rodada *r, Valor manilha);

#endif