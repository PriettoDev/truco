#ifndef CONFERENCIA_H
#define CONFERENCIA_H

/*
 * conferencia.h - verifica no inicio do jogo se alguma area de tela cobre
 * outra que precisa ficar visivel ou clicavel. Fica num arquivo proprio
 * porque precisa enxergar a cena, as cartas e o HUD ao mesmo tempo.
 *
 * Regras conferidas:
 *  - paineis do bar (clicaveis) nao ficam atras de personagem nem da mesa;
 *  - o HUD nao cobre a mao, a vira/monte, os rostos nem os paineis do bar.
 *
 * So avisa no terminal; nunca trava o jogo.
 */

/* Devolve 1 se esta tudo livre, 0 se achou alguma sobreposicao. */
int conferir_posicoes(void);

#endif
