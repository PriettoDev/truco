#ifndef MESA_H
#define MESA_H

#include "jogador.h"

#define MAX_JOGADOR 4
#define MAOS_POR_RODADA 3
#define PONTOS_PARA_VENCER 12
#define EMPATE_MAO -1   // valor que significa "a mao empatou"

/* ---------- lista duplamente encadeada (precisa vir ANTES da Mesa) ---------- */
typedef int tp_item;

typedef struct tp_no_aux {
    struct tp_no_aux *ant;
    tp_item info;
    struct tp_no_aux *prox;
} tp_no;

typedef struct {
    tp_no *ini;
    tp_no *fim;
} tp_listad;

tp_listad *inicializa_listad(void);
int listad_vazia(tp_listad *lista);
tp_no *aloca(void);
int insere_listad_no_fim(tp_listad *lista, tp_item e);
int remove_listad(tp_listad *lista, tp_item e);
void imprime_listad(tp_listad *lista, int ordem);
tp_no *busca_listade(tp_listad *lista, tp_item e);
tp_listad *Destroi_listad(tp_listad *lista);

/* ---------- mesa ---------- */
typedef struct {
    Jogador jogadores[MAX_JOGADOR];
    tp_listad *ordem;
    int numeroDaRodada;
    int inicioDaRodada;   // indice de quem abriu a rodada atual (-1 = ainda nao definido)
    int pontosDupla1;
    int pontosDupla2;
} Mesa;

int inicializarMesa(Mesa *mesa);
int inserirJogadorNaOrdem(Mesa *mesa, int indice);
void girarMesa(Mesa *mesa);
void imprimirOrdemDaMao(Mesa *mesa);
void destruirMesa(Mesa *mesa);
int definirPrimeiroDaMao(Mesa *mesa, int indice);
void prepararProximaMao(Mesa *mesa, int indiceVencedor);
int iniciarNovaRodada(Mesa *mesa);

#endif
