#ifndef TELAS_PARTIDA_H
#define TELAS_PARTIDA_H

/*
 * telas_partida.h - telas que cobrem a mesa no passa a vez: a cobertura
 * entre uma vez e outra, o resultado da rodada e os avisos de fim de mao e
 * de partida.
 *
 * So desenham: quem decide quando cada uma aparece e partida_gui.c.
 * Recebem nomes e numeros prontos para nao depender da struct da partida.
 */
#include "raylib.h"

/* Plaquinha "Rodada:" a esquerda da cruz, longe dos rostos, da mao e das
   cartas jogadas (conferencia.c confere). Na resposta ao truco, o mesmo
   lugar lembra quem pediu e o que. */
#define TELAS_RESULTADO_RODADA ((Rectangle){ 440, 630, 310, 140 })

/* Resultado da rodada ja convertido para quem esta vendo. */
typedef enum { RODADA_NOS, RODADA_ELES, RODADA_EMPATE } ResultadoVisto;

/* Cobre a tela inteira para o jogador da vez pegar o computador sem que
   ninguem veja as cartas. tempo = segundos desde que a cobertura abriu: o
   convite para clicar aparece aos poucos ate a trava de clique acabar. */
void telas_cobertura(int assento, const char *nome, float tempo, float trava);

/* "Rodada: Nos/Eles/Empate" durante a apuracao. tempo = segundos desde
   que a apuracao comecou (a plaquinha aparece aos poucos). */
void telas_resultado_rodada(ResultadoVisto resultado, float tempo);

/* Fim de mao por cima da mesa escurecida. duplaVencedora = "Fulano e
   Beltrano", ou NULL se a mao empatou (ninguem pontua). duplaCorreu: NULL,
   ou a dupla que correu do truco (a mao acabou antes das 3 rodadas). */
void telas_fim_mao(const char *duplaVencedora, const char *duplaCorreu, int pontos, float tempo, float trava);

/* --- truco. nomeValor vem de partida_nome_do_valor ("TRUCO", "SEIS"...). */

/* "TRUCO!" grande por cima da mesa, com o nome de quem pediu. tempo =
   segundos de anuncio (a palavra entra crescendo). */
void telas_anuncio_pedido(const char *nomePediu, const char *nomeValor, float tempo);

/* Passa o computador para quem responde: "<quem pediu> pediu TRUCO!" e
   "VEZ DE <quem responde>", com o personagem dele, como a cobertura. */
void telas_cobertura_resposta(int assentoResponde, const char *nomeResponde, const char *nomePediu,
                              const char *nomeValor, float tempo, float trava);

/* Lembrete durante a resposta, no lugar da plaquinha da rodada. */
void telas_pedido_pendente(const char *nomePediu, const char *nomeValor);

/* Fim de partida: os dois vencedores de frente, o nome da dupla e o placar
   final. assentoA/assentoB sao os assentos da dupla vencedora. */
void telas_fim_partida(int assentoA, int assentoB, const char *duplaVencedora,
                       int pontosVencedora, int pontosPerdedora, float tempo, float trava);

#endif
