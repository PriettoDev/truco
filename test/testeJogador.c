#include <stdio.h>
#include "../src/jogador.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(){
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    char nomeOrigem1[30] = "Pedro";
    int dupla1 = 1;
    char nomeOrigem2[30] = "Lucas";
    int dupla2 = 2;
    char nomeOrigem3[30] = "Manu";
    int dupla3 = 1;
    
    Jogador jogador1 = jogador_criar(nomeOrigem1, dupla1);
    Jogador jogador3 = jogador_criar(nomeOrigem2, dupla2);
    Jogador jogador2 = jogador_criar(nomeOrigem3, dupla3);

    Carta c1 = {OUROS, DOIS};
    Carta c2 = {COPAS, QUATRO};
    Carta c3 = {PAUS, AS};
    Carta cartaJogada1 = {PAUS, CINCO};
    Carta cartaJogada2 = {PAUS, CINCO};
    Carta cartaJogada3 = {PAUS, CINCO};

    jogador_distribuir_cartas(&jogador1, c1, c2, c3);
    jogador_distribuir_cartas(&jogador2, c1, c2, c3);
    jogador_distribuir_cartas(&jogador3, c1, c2, c3);
    int inteiroSaida1 = jogador_jogar_carta(&jogador1, 0, &cartaJogada1);
    int inteiroSaida2 = jogador_jogar_carta(&jogador2, 1, &cartaJogada2);
    int inteiroSaida3 = jogador_jogar_carta(&jogador3, 2, &cartaJogada3);
    printf("\n--- pos = 0 ---\n");
    printf("retorno: %d\n", inteiroSaida1);
    printf("\n--- pos = 1 ---\n");
    printf("retorno: %d\n", inteiroSaida2);
    printf("\n--- pos = 2 ---\n");
    printf("retorno: %d\n", inteiroSaida3);
    printf("\nCarta jogada pos=0:");
    carta_imprimir(cartaJogada1);
    printf("\nCarta jogada pos=1:");
    carta_imprimir(cartaJogada2);
    printf("\nCarta jogada pos=2:");
    carta_imprimir(cartaJogada3);
    printf("\n");
    printf("\nJogador 1:");
    jogador_imprimir(&jogador1);
    printf("\nJogador 2:");
    jogador_imprimir(&jogador2);
    printf("\nJogador 3:");
    jogador_imprimir(&jogador3);
}