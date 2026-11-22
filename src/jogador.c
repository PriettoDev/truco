#include <stdio.h>
#include <string.h>
#include "jogador.h"

Jogador jogador_criar(char nome [], int dupla, int assento){
    Jogador novoJogador;
    strncpy(novoJogador.nome, nome, sizeof(novoJogador.nome) - 1);   // o "strncpy" me permite a nunca deixar que o usuario passe de 29 letras, assim nn da bugs
    novoJogador.nome[sizeof(novoJogador.nome) - 1] = '\0';           // garante o fim da string
    novoJogador.dupla = dupla;
    novoJogador.assento = assento;
    novoJogador.qtdCartas = 0;

    return novoJogador;
}

int jogador_assento(const Jogador *j) {
    if (j == NULL) {
        return -1;
    }
    return j->assento;
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

int jogador_jogar_carta(Jogador *j, int pos, Carta *c){
    if(pos<0 || pos>=j->qtdCartas)
        return 0;

    *c = j->mao[pos];

    for(int i = pos; i<j->qtdCartas-1; i++){
        j->mao[i] = j->mao[i+1];
    }
    j->qtdCartas--;

    return 1;
}