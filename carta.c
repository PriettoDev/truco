#include <stdio.h>
#include "carta.h"


static const char *nome_naipe[] = {"♦", "♠", "♥" , "♣"};
const char *naipe_texto(Naipe n) {
    if (n < OUROS || n > PAUS) {
        return "?";
    }
    return nome_naipe[n];
}

static const char *nome_valor[] = { "4", "5", "6" , "7" , "Q", "J", "K", "A", "2", "3" };
const char *valor_texto (Valor v) {
    if (v < QUATRO || v > TRES) {
        return "?";
    }
    return nome_valor[v];
}

void carta_imprimir(Carta c){
    printf("| %s %s | ", valor_texto(c.valor), naipe_texto(c.naipe));
    
}

Valor calcular_manilha(Carta vira){
    return (vira.valor + 1) % 10;
}