#include <stdio.h>
#include "baralho.h"

int main(void) {

    Baralho b;
    Carta c;

    inicializaBaralho(&b);
    criaBaralho(&b);

    printf("Baralho criado:\n\n");
    imprimeBaralho(b);

    printf("\nTopo antes da retirada: %d\n", b.topo);

    if (retiraCarta(&b, &c)) {
        printf("Carta retirada: ");
        carta_imprimir(c);
        printf("\n");
    }

    printf("Topo depois da retirada: %d\n", b.topo);

    return 0;
}
