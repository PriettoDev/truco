#include <stdio.h>
#include "fila.h"

int main(){
    
    tp_fila trucofila;
    Mesa mesa;
    int i;
    
    // Instruções sobre como funciona Dupla/Jogador //
    
    printf("O Primeiro nome digitado sera o 1° Jogador, que eh da Dupla 1\n");
    printf("O Segundo nome digitado sera o 2° Jogador, que eh da Dupla 2\n");
    printf("O Terceiro nome digitado sera o 3° Jogador, que eh da Dupla 1\n");
    printf("O Quarto nome digitado sera o 4° Jogador, que eh da Dupla 2\n");
    
    // Definindo o nome de cada jogador para o jogo//
    
    for (i = 0; i < 4; i++){
        printf("\nNome do jogador %d: \n", i + 1);
        scanf(" %[^\n]", mesa.jogadores[i].nome);
    }
    
    // Definindo a dupla de cada jogador //
    mesa.jogadores[0].dupla = 1;
    mesa.jogadores[1].dupla = 2;
    mesa.jogadores[2].dupla = 1;
    mesa.jogadores[3].dupla = 2;


    inicializaFILA(&trucofila);

    // Inserindo os jogadores na ordem do jogo //

    insereFila(&trucofila, 1);
    insereFila(&trucofila, 2);
    insereFila(&trucofila, 3);
    insereFila(&trucofila, 4);
    
    // Jogadores que estão no jogo //
    
    for (i = 0; i < 4; i++){
        printf("\nJogador %d: %s - Dupla %d\n",
            i + 1,
            mesa.jogadores[i].nome,
            mesa.jogadores[i].dupla
        );
    }

    // Ordem dos jogadores de cada mão //
    // PRIMEIRA MAO //

    PrimeiraMao(&trucofila, mesa);
    
    // SEGUNDA MAO //
    
    SegundaMao(&trucofila, mesa);
    
    // TERCEIRA MAO //
    
    TerceiraMao(&trucofila, mesa);
}