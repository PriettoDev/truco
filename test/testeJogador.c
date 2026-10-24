#include <stdio.h>
#include "../src/jogador.h"

int main(){
    char nomeOrigem[30] = "Pedro";
    int dupla = 1;
    
    Jogador jogador1 = jogador_criar(nomeOrigem, dupla);

    Carta c1 = {OUROS, DOIS};
    Carta c2 = {COPAS, QUATRO};
    Carta c3 = {PAUS, AS};

    jogador_distribuir_cartas(&jogador1, c1, c2, c3);
    jogador_imprimir(&jogador1);
}