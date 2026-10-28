#include <stdio.h>
#include "mesa.h"

// avanca uma posicao na fila circular, voltando ao inicio quando chega no final
static int proximaPosicao(int pos) {
    if (pos == MAX_JOGADOR - 1) return 0;
    return pos + 1;
} //marquei esta função como static pois ela é um detalhe interno da fila circular, ela não será chamada por mais ninguem

void inicializarMesa(Ordem *o) {
    o->ini = o->fim = MAX_JOGADOR - 1;
}

int mesaVazia(Ordem *o) {
    if (o->ini == o->fim) {
        return 1;
    }
    return 0;
}

int mesaCheia(Ordem *o) {
    if (proximaPosicao(o->fim) == o->ini) {
        return 1;
    }
    return 0;
}
//AJUSTAR
int inserirMesa(Ordem *o, Jogador *jogador) {
    if (mesaCheia(o) || jogador == NULL) {
        return 0;
    }
    o->fim = proximaPosicao(o->fim);
    o->posicoes[o->fim] = jogador;
    return 1;
}

int removerMesa(Ordem *o, Jogador **jogador) {
    if (mesaVazia(o) || jogador == NULL) {
        return 0;
    }
    o->ini = proximaPosicao(o->ini);
    *jogador = o->posicoes[o->ini];
    return 1;
}
//AJUSTAR
int tamanhoMesa(Ordem *o) {
    if (mesaVazia(o)) {
        return 0;
    }
    if (o->ini < o->fim) {
        return o->fim - o->ini;
    }
    return MAX_JOGADOR - 1 - o->ini + o->fim + 1;
}

// gira a ordem de jogo: quem era "mao" vai para o fim, e o proximo jogador assume
void girarMesa(Ordem *o) {
    Jogador *jogador;
    if (removerMesa(o, &jogador)){
        inserirMesa(o, jogador);
    }
}

// mostra a ordem de jogo desta mao. Recebe a Mesa POR VALOR de proposito:
// consome uma COPIA da ordem para percorrer, sem alterar a ordem real da mesa
void imprimirOrdemDaMao(const Mesa *mesa) {
    Ordem copia = mesa->ordem;
    Jogador *jogador;
    printf("\nOrdem de jogo desta mao:\n");
    while (removerMesa(&copia, &jogador)) {
        printf("Jogador: %s (Dupla %d)\n", jogador->nome, jogador->dupla);
    }
}
//as três funções que tinha antes "PrimeiraMao,SegundaMao e TerceiraMao" eu unifiquei tudo aqui em imprimirOrdemDaMao, como as três faziam a mesma coisa botei tudo em uma só funçãogenerica chamada 3 vezes facilita o trabalho e deixa mais enxuto o codigo