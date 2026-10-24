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
    printf(" | %s %s | ", valor_texto(c.valor), naipe_texto(c.naipe));
    
}

/* 

(Valor manilha_da_vira): tem o papel de manter a sequencia ciclica dos valores, 
mantendo a manilha como a proxima carta que continuará a sequencia. 

*/

// ----------------------------------------------------------------------------------------------------

Valor manilha_da_vira(Carta vira) {
    return (Valor)((vira.valor + 1) % NUM_VALORES);
}

/* 

Por que retornamos um Valor e nao uma Carta:

A manilha nunca será uma carta especifica, mas sim um valor.
Apos uma VALOR se tornar a manilha, os 4 naipes viram a manilha. Escolher uma carta aqui
seria um erro de regra do jogo, pois teriamos que escolher uma carta (valor) + naipe.

*/


/* 

Calculo ciclico: Assim como um relogio, aqui apliquei a mesma logica para o retorno 
ciclico dos valores das cartas. Por exemplo: (9 + 1) % 10 = 0 -> 9 Seria o index para a carta TRES, 
+1 seria a proxima com isso, retornariamos para a primeira carta da sequencia, que é a de index 0 (QUATRO), a mais fraca.

*/

// ----------------------------------------------------------------------------------------------------

int carta_eh_manilha(Carta c, Valor manilha) {
    return c.valor == manilha;
}

/* 

(int carta_eh_manilha(Carta c, Valor manilha)): Reconhecendo se uma carta é manilha.

Aqui estamos utilizando uma funçåo que retorna quando uma valor é manilha,
e isso se propaga para todos os naipes e nao apenas para a carta especifica (com valor + naipe).
A manilha vem de fora como argumento e calculada pela funçåo manilha_da_vira. 

*/

int carta_forca(Carta c, Valor manilha) {
    if (carta_eh_manilha(c, manilha)) {
        return NUM_VALORES + c.naipe;
    }
    return c.valor;
}

/*
Aqui é aonde vamos definir a força da carta levando a manilha em conta. Antes estávamos
apenas ordenando os naipes e os valores nos enums; a partir desse momento, essa ordem vira
um número único de força, para que comparar duas cartas seja comparar dois inteiros.

Exemplo caso a carta seja manilha (carta_eh_manilha): 

* Ouros ->  (Força = NUM_VALORES + naipe) -> Força Ouros = (10) + (0) = Força Ouros = 10
* Paus ->  (Força = NUM_VALORES + naipe) -> Força Paus = (10) + (3) = Força Paus = 13

Comparando: Força Paus > Força Ouros = 13 > 10


O 10 não é por acaso: a maior carta comum vale 9 (TRES), então somar NUM_VALORES
garante que até a manilha mais fraca (Força Manilha Ouros = 10 + 0) passe por cima de todas as comuns (Não manilha).

Return c.valor Caso a carta não seja manilha: Aqui seria um retorno simples que ja utiliza da ordem estabelecida
pelo ENUM para que as cartas que não são manilha, tenham a sua força contabilizada. (Ja estao em ordem).

Exemplo: 3 de Paus -> c.valor é TRES, que ocupa a posição 9 no enum -> return 9;

*/

int carta_comparar(Carta a, Carta b, Valor manilha) {
    int fa = carta_forca(a, manilha);
    int fb = carta_forca(b, manilha);
    if (fa > fb) return  1;
    if (fa < fb) return -1;
    return 0;
}

/*

Nessa função temos a comeparação entre as cartas da rodada. Aqui vamos registrar valores em duas 
variaveis fa e fb (Força de A e Força de B), dessa forma podemos fazer uma comparação tripla de 
A ganha, B ganha ou Empate. 


A logica de ter duas variaveis intenas nessa função é que podemos registrar a força de cada carta
recebida pelo parametro Carta a e Carta b, para que entao possamos apenas comparar cada uma de maneira 
que possamos ter um "Three-way comparison". O motivo é que dessa forma nao precisaremos chamar a função 
int carta_forca(Carta c, Valor manilha), todas as vezes.

int fa = carta_forca(a, manilha); -> Registra o valor da força da Carta a recebida por parametro em fa
int fb = carta_forca(b, manilha); -> Registra o valor da força da Carta b recebida por parametro em fb

*Comparação: Three-way comparison

if (fa > fb) return  1; -> Retorna 1 caso fa seja maior que fb
if (fa < fb) return -1; -> Retorna -1 caso fb seja maior que fa
return 0; -> Retorna 0 caso seja empate comparativo

*/