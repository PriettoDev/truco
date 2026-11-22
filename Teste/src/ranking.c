#include <stdio.h>
#include "arvore.h"
#include "ranking.h"

// a arvore guarda so um int, entao junto as maos e o assento
// ex: assento 2 com 5 maos vira 52
#define BASE_CHAVE 10

// Junta as maos e o assento num numero so (ex: 5 maos, assento 2 = 52)
static int montar_chave(int maosGanhas, int assento) {
    return maosGanhas * BASE_CHAVE + assento;
}

// Zera as maos ganhas dos 4 jogadores
void ranking_iniciar(Ranking *r) {
    if (r == NULL) return;
    for (int i = 0; i < RANKING_NUM_JOGADORES; i++) {
        r->maosGanhas[i] = 0;
    }
}

// soma 1 para os dois jogadores da dupla que ganhou a mao
void ranking_registrar_mao(Ranking *r, int duplaVencedora) {
    if (r == NULL) return;
    if (duplaVencedora != 0 && duplaVencedora != 1) return;   // empate

    for (int assento = 0; assento < RANKING_NUM_JOGADORES; assento++) {
        if (assento % 2 == duplaVencedora) {
            r->maosGanhas[assento]++;
        }
    }
}

// percorre em ordem e separa o assento e as maos da chave
static void mural_em_ordem(const tp_no *p, const Jogador jogadores[4], int *posicao) {
    if (p == NULL) return;

    mural_em_ordem(p->esq, jogadores, posicao);

    int assento = p->info % BASE_CHAVE;
    int maos = p->info / BASE_CHAVE;
    printf("%d. %s (Dupla %d) - %d mao(s) ganha(s)\n", *posicao,
           jogadores[assento].nome, jogadores[assento].dupla, maos);
    (*posicao)++;

    mural_em_ordem(p->dir, jogadores, posicao);
}

// Monta a arvore, mostra o mural e libera a memoria
void ranking_exibir_mural(const Ranking *r, const Jogador jogadores[4]) {
    tp_arvore raiz = inicializa_arvore();
    int posicao = 1;

    if (r == NULL || jogadores == NULL) return;

    // coloca os 4 jogadores na arvore
    for (int assento = 0; assento < RANKING_NUM_JOGADORES; assento++) {
        if (!insere_no(&raiz, montar_chave(r->maosGanhas[assento], assento))) {
            printf("Erro: sem memoria.\n");
            destroi_arvore(&raiz);
            return;
        }
    }

    printf("\n========== MURAL DO BAR ==========\n");
    mural_em_ordem(raiz, jogadores, &posicao);

    destroi_arvore(&raiz);
}