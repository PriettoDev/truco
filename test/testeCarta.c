#include <stdio.h>
#include "../src/carta.h"

int main(void) {
    Carta c = {COPAS, DAMA};
    Carta d = {ESPADAS, TRES};
    Carta f = {OUROS, DOIS};
    Carta n = {COPAS, DOIS};

    printf("Mao: ");
    carta_imprimir(c);  
    printf(" | ");
    carta_imprimir(d);  
    printf(" | ");
    carta_imprimir(f);
    printf(" | ");
    carta_imprimir(n);
    printf("\n");
   
    return 0;
}