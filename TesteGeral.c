#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "carta.h"
#include "baralho.h"
#include "jogador.h"
#include "mesa.h"


//TesteGeral: executa todo o fluxo de inicialização e dinâmica do jogo
int main(){
    srand(time(NULL)); //Semente para embaralhar aleatoriamente

    printf("\n  INICIANDO O TESTE GERAL DO TRUCO    \n");

    //Inicialização e criando o Baralho
    Baralho baralho;
    inicializaBaralho(&baralho);
    criaBaralho(&baralho);
    embaralhaBaralho(&baralho);

    printf("Baralho de %d cartas criado e embaralhado.\n\n", baralho.topo + 1);

    //Mesa e Jogadores
    Mesa mesa;
    inicializarMesa(&mesa.ordem);
    
    printf("--- CADASTRO DOS JOGADORES ---\n");
    char nomeDigitado[30];

    for(int i=0; i<4; i++) {
        int dupla = (i % 2 == 0) ? 1 : 2;

        printf("Digite o nome do Jogador %d (Dupla %d): ", i + 1, dupla);
        scanf(" %[^\n]", nomeDigitado);

    // Cria e guarda o jogador usando a função da biblioteca jogador.h[cite: 9, 10]
        mesa.jogadores[i] = jogador_criar(nomeDigitado, dupla);
        inserirMesa(&mesa.ordem, i);
    }

    printf("\n--- JOGADORES NA MESA ---\n");
    for (int i = 0; i < 4; i++) {
        printf("Posicao %d: %s (Dupla %d)\n", i + 1, mesa.jogadores[i].nome, mesa.jogadores[i].dupla);
    }

    //PARTE PARA CONSIDERAR RETIRAR
    retiraCarta(&baralho, &mesa.vira);
    Valor valorManilha = calcular_manilha(mesa.vira);

    printf("\nVIRA REVELADO DA MESA:  ");
    carta_imprimir(mesa.vira);
    printf("\nMANILHA DA RODA: %s\n", valor_texto(valorManilha));

    //EXCLUIR PARTE DE CIMA

    //Distribuição de cartas do baralho
    printf("\n\nDistribuindo 3 cartas para cada jogador...\n\n");
    for(int i=0; i<4; i++){
        Carta c1, c2, c3;
        retiraCarta(&baralho, &c1);
        retiraCarta(&baralho, &c2);
        retiraCarta(&baralho, &c3);
        jogador_distribuir_cartas(&mesa.jogadores[i], c1, c2, c3);
    }

    printf("    CARTAS DISTRIBUIDAS E GUARDADAS \n");
    for(int i=0; i<4; i++){
        jogador_imprimir(&mesa.jogadores[i]);
        printf("\n--------------------\n");
    }

    //Ordem dos jogadores da mesa
    imprimirOrdemDaMao(mesa);

    printf("\n\n    CONFIGURACAO DOS JOGADORES NA MESA  ");
    girarMesa(&mesa.ordem);
    printf("\nO 'Mao' acabou de jogar e foi enviado para o fim da roda!\n");
    
    imprimirOrdemDaMao(mesa);

    printf("\nDISTRIBUICAO E CONEXAO DE ESTRUTURAS CONCLUIDA!  \n");
    return 0;
}