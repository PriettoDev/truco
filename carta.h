#ifndef CARTA_H 
#define CARTA_H


/* Naipes na ordem de desempate entre manilhas: OUROS ate PAUS.
   A ordem daqui e regra do jogo. Nao reordenar. */

typedef enum {
    OUROS, // 0
    ESPADAS, // 1
    COPAS, // 2
    PAUS // 3
} Naipe;

/* -> A ordem daqui e regra do jogo. Não reordenar! */

typedef enum {
    
    QUATRO, // 0 
    CINCO, // 1
    SEIS, // 2
    SETE, // 3
    DAMA, // 4
    VALETE, // 5
    REI, // 6
    AS, // 7
    DOIS, // 8
    TRES  // 9
} Valor;  

typedef struct {
    Naipe naipe;
    Valor valor;
} Carta;

const char *naipe_texto(Naipe n);
const char *valor_texto (Valor v);
void carta_imprimir(Carta c);

Valor calcular_manilha(Carta vira);

#endif