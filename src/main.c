#ifdef _WIN32
#include <windows.h> //O arquivo carta.c foi salvo em UTF-8 (codificação que representa corretamente ♦ ♠ ♥ ♣), mas o terminal do Windows espera outra codificação de caracteres. Por isso adicionamos a biblioteca windows.h que força a pagina de código do console pra UTF-8, assim o terminal vai mostrar corretamente os simbolos das cartas.
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "carta.h"
#include "baralho.h"
#include "jogador.h"
#include "mesa.h"

//TesteGeral: executa todo o fluxo de inicializacao e dinamica do jogo
int main(){
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    srand(time(NULL)); //Semente para embaralhar aleatoriamente

    printf("\n  INICIANDO O TESTE GERAL DO TRUCO    \n");

    //Inicializacao e criacao do Baralho
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
        scanf(" %29[^\n]", nomeDigitado);

        // Cria e guarda o jogador usando a funcao da biblioteca jogador.h
        mesa.jogadores[i] = jogador_criar(nomeDigitado, dupla);
        inserirMesa(&mesa.ordem, i);
    }

    printf("\n--- JOGADORES NA MESA ---\n");
    for (int i = 0; i < 4; i++) {
        printf("Posicao %d: %s (Dupla %d)\n", i + 1, mesa.jogadores[i].nome, mesa.jogadores[i].dupla);
    }

    //Distribuicao de cartas do baralho (PRIMEIRO as maos, depois a vira)
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

    //So depois de distribuir as maos, a vira e revelada e a manilha calculada
    retiraCarta(&baralho, &mesa.vira);
    Valor valorManilha = manilha_da_vira(mesa.vira);

    printf("\nVIRA REVELADO DA MESA:  ");
    carta_imprimir(mesa.vira);
    printf("\nMANILHA DA RODA: %s\n", valor_texto(valorManilha));

    //Ordem dos jogadores da mesa
    imprimirOrdemDaMao(mesa);

    printf("\n\n    CONFIGURACAO DOS JOGADORES NA MESA  ");
    girarMesa(&mesa.ordem);
    printf("\nO 'Mao' acabou de jogar e foi enviado para o fim da roda!\n");

    imprimirOrdemDaMao(mesa);

    printf("\nDISTRIBUICAO E CONEXAO DE ESTRUTURAS CONCLUIDA!  \n");
    return 0;
}