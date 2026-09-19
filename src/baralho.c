#include <stdio.h>
#include "baralho.h"

//inicializa o baralho sem cartas disponíveis
void inicializaBaralho(Baralho *b) {
    b->topo = -1;
}

//cria as 40 cartas do baralho a partir dos naipes e valores
void criaBaralho(Baralho *b) {
    int posicao = 0;

    for (int n = OUROS; n <= PAUS; n++) { //percorre os 4 naipes
        for (int v = QUATRO; v <= TRES; v++) { //percorre os 10 valores

            b->cartas[posicao].naipe = n;
            b->cartas[posicao].valor = v;

            posicao++;
        }
    }

    b->topo = MAX_CARTAS - 1;
}

//verifica se não existem mais cartas disponíveis no baralho
int baralhoVazio(Baralho *b) {
    if (b->topo == -1) //se o topo for -1, não existe nenhuma posição com carta disponível
        return 1;

    return 0;
}

//retira uma carta do topo do baralho
int retiraCarta(Baralho *b, Carta *c) { //vai alterar o topo e entrega pra quem chamou qual carta foi retirada
    if (baralhoVazio(b)) //se não existe carta não acessa o vetor
        return 0;

    *c = b->cartas[b->topo]; //pegar as cartas (39) 
    b->topo--; // passa para o próximo (diminui o topo)

    return 1;
}

//exibe as cartas disponíveis, começando pelo topo
void imprimeBaralho(Baralho b) { //a função só consulta o baralho para imprimir, ela não precisa modificar o original, baralho por valor
    for (int i = b.topo; i >= 0; i--) {
        carta_imprimir(b.cartas[i]);
        printf("\n");
    }
}
