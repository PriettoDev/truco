#ifndef PONTUACAO_H
#define PONTUACAO_H

/*
Pontuação da partida. Aqui vai somar os pontos de cada mão e decidir quando a partida acaba, no caso 12 pontos, e quem venceu.
Criei este modulo para que a interface grafica possa usar essas regras diretamente deste modulo. Pois antes essas regras estavam dentro do main, e a interface tinha uma copia (muita bagunça). Com o modulo a interface e o terminal usam o mesmo codigo (Principio de responsabilidade unica - SRP e don't repeat youself - DRY).
Este modulo só vai ler/escrever o placar da Mesa/ e le o resultado da Mao/.
*/
#include "mesa.h"
#include "mao.h"


int pontuacao_somar_mao(Mesa *mesa, const Mao *mao);
//Essa função soma os pontos da mao da dupla vencedora, sem passar de "pontos_para_vencer". A mao empatada ou sem vencedor não pontua e devolve quantos pontos entrarem no placar.

int pontuacao_dupla_vencedora(const Mesa *mesa);
//Essa função devolve 1 ou 2, que seria a dupla que chegou no "pontos_para_vencer" ou 0 se ninguem chegou.

int pontuacao_partida_terminou(const Mesa *mesa);
//1 se alguma dupla fechou a partida, ou 0 se nenhuma dupla fechou.

#endif