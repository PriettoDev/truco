#ifndef DESENHO_CARTAS_H
#define DESENHO_CARTAS_H

/*
 * desenho_cartas.h - faces, versos, vira e mao do jogador.
 *
 * As faces sao desenhadas em codigo (so o verso e imagem): sao 40 cartas que
 * mudam so no valor e no naipe, e o texto vem de valor_texto/naipe_texto do
 * modulo carta, entao a tela nunca diverge da regra.
 * O arquivo se chama desenho_cartas para nao confundir com src/carta.h.
 */
#include "raylib.h"
#include "carta.h"

#define DESENHO_CARTA_PROPORCAO 1.5f   /* altura = 1,5 x largura (baralho comum) */

/* Mao do jogador: centralizada embaixo, um pouco para fora da tela, para
   parecer que esta na mao e nao em cima da mesa. */
#define DESENHO_MAO_LARGURA_CARTA 200.0f
#define DESENHO_MAO_ESPACO        230.0f   /* distancia entre os centros */
#define DESENHO_MAO_TOPO          820.0f
#define DESENHO_MAO_ELEVACAO       60.0f   /* quanto a carta sobe com o mouse */
#define DESENHO_MAO_SUAVIDADE      14.0f   /* maior = chega mais rapido */

/* Monte e vira lado a lado, a direita do centro: no truco paulista a vira
   sai do monte e fica junto dele, e o meio da mesa fica livre para as cartas
   jogadas. A borda direita do monte (x 1319) fica uns 40 px antes da caixa
   do personagem da direita (x 1360). */
#define DESENHO_VIRA_CENTRO   ((Vector2){ 1150, 680 })
#define DESENHO_VIRA_LARGURA  100.0f
#define DESENHO_MONTE_CENTRO  ((Vector2){ 1260, 680 })
#define DESENHO_MONTE_CAMADAS 4        /* versos empilhados para dar volume */

/* Versos diante dos outros 3 jogadores. "graus" e a rotacao do leque inteiro:
   cada jogador segura as cartas virado para o centro da mesa. */
#define DESENHO_VERSO_LARGURA         80.0f
#define DESENHO_VERSOS_FRENTE_CENTRO   ((Vector2){  960, 650 })
#define DESENHO_VERSOS_FRENTE_GRAUS    180.0f
#define DESENHO_VERSOS_ESQUERDA_CENTRO ((Vector2){  330, 790 })
#define DESENHO_VERSOS_ESQUERDA_GRAUS   90.0f
#define DESENHO_VERSOS_DIREITA_CENTRO  ((Vector2){ 1590, 790 })
#define DESENHO_VERSOS_DIREITA_GRAUS   -90.0f

/* Face de uma carta no retangulo dado (sem rotacao). */
void desenho_carta_face(Carta carta, Rectangle area);

/* Mao do jogador: atualizar uma vez por quadro (anima a carta sob o mouse)
   e depois desenhar. mouse em coordenadas logicas, dt em segundos. */
void desenho_mao_atualizar(int quantidade, Vector2 mouse, float dt);
void desenho_mao(const Carta cartas[], int quantidade);

void desenho_vira(Carta vira);
void desenho_monte(void);

/* Areas que a interface nunca pode cobrir (usadas na conferencia do inicio):
   a mao inteira, incluindo a altura da carta levantada, e vira + monte. */
Rectangle desenho_area_mao(int quantidade);
Rectangle desenho_area_vira_e_monte(void);

/* Versos diante de cada um dos outros 3 jogadores (0 a 3 cartas cada). */
void desenho_versos_dos_outros(int qtdDireita, int qtdFrente, int qtdEsquerda);

#endif
