#ifndef TELAS_PARTIDA_H
#define TELAS_PARTIDA_H

/*
 * telas_partida.h - telas que cobrem a mesa no passa a vez: a cobertura
 * entre uma vez e outra e os avisos de fim de mao e de partida.
 *
 * So desenham: quem decide quando cada uma aparece e partida_gui.c.
 * Recebem nomes e numeros prontos para nao depender da struct da partida.
 */

/* Cobre a tela inteira para o jogador da vez pegar o computador sem que
   ninguem veja as cartas. tempo = segundos desde que a cobertura abriu: o
   convite para clicar aparece aos poucos ate a trava de clique acabar. */
void telas_cobertura(int assento, const char *nome, float tempo, float trava);

/* PROVISORIO (as telas de verdade vem no commit de apuracao): faixa no
   meio da tela avisando que e preciso clicar para seguir. */
void telas_aviso_provisorio(const char *titulo, const char *convite);

#endif
