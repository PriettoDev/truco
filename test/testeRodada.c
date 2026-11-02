#include <stdio.h>
#include "../src/rodada.h"

int main(void) {
    Rodada r;
    Carta c = {OUROS, DOIS};

    rodada_iniciar(&r);
    printf("assento 0 (esperado 1): %d\n", rodada_jogar(&r, c, 0, 0));
    printf("assento 1 (esperado 1): %d\n", rodada_jogar(&r, c, 1, 1));
    printf("completa antes das 4 (esperado 0): %d\n", rodada_completa(&r));
    printf("assento 2 (esperado 1): %d\n", rodada_jogar(&r, c, 2, 0));
    printf("assento 3 (esperado 1): %d\n", rodada_jogar(&r, c, 3, 1));
    printf("completa com 4 (esperado 1): %d\n", rodada_completa(&r));
    printf("5a carta, mesa cheia (esperado 0): %d\n", rodada_jogar(&r, c, 0, 0));

    rodada_iniciar(&r);
    printf("assento 2 (esperado 1): %d\n", rodada_jogar(&r, c, 2, 0));
    printf("assento 2 de novo (esperado 0): %d\n", rodada_jogar(&r, c, 2, 0));
    printf("qtd (esperado 1): %d\n", r.qtd);

    rodada_iniciar(&r);
    printf("assento 4 (esperado 0): %d\n", rodada_jogar(&r, c, 4, 0));
    printf("assento -1 (esperado 0): %d\n", rodada_jogar(&r, c, -1, 0));
    printf("dupla 2 (esperado 0): %d\n", rodada_jogar(&r, c, 0, 2));
    printf("assento 0 na dupla 1 (esperado 0): %d\n", rodada_jogar(&r, c, 0, 1));
    return 0;
}