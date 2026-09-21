#include <stdio.h>
#include "mesa.h"

int main(void) {
    Mesa mesa;
    int duplas[4] = {1, 2, 1, 2};

    printf("O Primeiro nome digitado sera o 1o Jogador, da Dupla 1\n");
    printf("O Segundo nome digitado sera o 2o Jogador, da Dupla 2\n");
    printf("O Terceiro nome digitado sera o 3o Jogador, da Dupla 1\n");
    printf("O Quarto nome digitado sera o 4o Jogador, da Dupla 2\n");

    for (int i = 0; i < 4; i++) {
        char nome[30];
        printf("\nNome do jogador %d: \n", i + 1);
        scanf("%29s", nome);
        mesa.jogadores[i] = jogador_criar(nome, duplas[i]);
    }

    inicializarMesa(&mesa.ordem);
    for (int i = 0; i < 4; i++) {
        inserirMesa(&mesa.ordem, i);
    }

    printf("\nPrimeira mao:");
    imprimirOrdemDaMao(mesa);

    girarMesa(&mesa.ordem);
    printf("\nSegunda mao:");
    imprimirOrdemDaMao(mesa);

    girarMesa(&mesa.ordem);
    printf("\nTerceira mao:");
    imprimirOrdemDaMao(mesa);

    return 0;
}