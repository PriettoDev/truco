#ifndef JOGADOR_H 
#define JOGADOR_H
#include "carta.h"

typedef struct {
    char nome [30];
    Carta mao [3];
    int qtdCartas;
    int dupla;
    int assento; // 0 a 3: lugar na mesa. Assentos 0 e 2 = dupla 1; 1 e 3 = dupla 2
} Jogador;


Jogador jogador_criar(char nome [], int dupla, int assento);
void jogador_distribuir_cartas(Jogador *j, Carta c1, Carta c2, Carta c3);
void jogador_imprimir(Jogador *j);
int jogador_jogar_carta(Jogador *j, int pos, Carta *c);
int jogador_assento(const Jogador *j);   // devolve 0 a 3, ou -1 se j for NULL 


#endif