#ifndef CARTA_H 
#define CARTA_H


/* Naipes na ordem de desempate entre manilhas: OUROS ate PAUS.
   A ordem daqui e regra do jogo. Nao reordenar. */

typedef enum {
    OUROS, // 0
    ESPADAS, // 1
    COPAS, // 2
    PAUS, // 3
    NUM_NAIPES  // 4 - Variavel de contagem, NAO e um naipe. Serve para o Baralho percorrer os naipes:
                //     em lacos use "< NUM_NAIPES", nunca "<=".
} Naipe;

/* Valores na ordem de força base: QUATRO (mais fraca) ate TRES (mais forte).
   Essa mesma ordem e a sequencia ciclica da manilha (4 5 6 7 Q J K A 2 3 e volta ao 4).
   A ordem daqui e regra do jogo. Nao reordenar. */

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
    TRES,  // 9
        NUM_VALORES // 10 - Variavel de contagem, NAO e uma carta. Usada no calculo ciclico
                //      da manilha e em lacos: use "< NUM_VALORES", nunca "<=".
} Valor;  

typedef struct {
    Naipe naipe;
    Valor valor;
} Carta;

const char *naipe_texto(Naipe n); // Traduz o enum (numero) em texto: COPAS -> "♥"
const char *valor_texto (Valor v); // Traduz o enum (numero) em texto: DAMA  -> "Q"
void carta_imprimir(Carta c); 
Valor manilha_da_vira(Carta vira); // vira 7 -> DAMA; vira 3 -> QUATRO
int carta_eh_manilha(Carta c, Valor manilha); // 1 se c.valor == manilha, senao 0
int carta_forca(Carta c, Valor manilha); // numero unico levando a manilha em conta: maior = mais forte
int carta_comparar(Carta a, Carta b, Valor manilha); // 1 se a vence, -1 se b vence, 0 empate

#endif