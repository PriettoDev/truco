#include <stdio.h>
#include <string.h>
#include "jogador.h"
#include "carta.h"

Jogador jogador_criar(char nome [], int dupla){
    Jogador novoJogador;
    strcpy(novoJogador.nome, nome);
    novoJogador.dupla = dupla;
    novoJogador.qtdCartas = 0;

    return novoJogador;
}

void jogador_distribuir_cartas(Jogador *j, Carta c1, Carta c2, Carta c3) {
    j->mao[0] = c1;
    j->mao[1] = c2;
    j->mao[2] = c3;
    j->qtdCartas = 3;
}

void jogador_imprimir(Jogador *j){
    printf("Jogador: %s\t Dupla: %d\n", j->nome, j->dupla);

    for (int i = 0; i < j->qtdCartas; i++) {
        carta_imprimir(j->mao[i]);
    }
}