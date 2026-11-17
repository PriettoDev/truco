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

/* Vira: a direita do centro, deixando o meio livre para as cartas jogadas. */
#define DESENHO_VIRA_CENTRO  ((Vector2){ 1340, 680 })
#define DESENHO_VIRA_LARGURA 110.0f

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

/* Versos diante de cada um dos outros 3 jogadores (0 a 3 cartas cada). */
void desenho_versos_dos_outros(int qtdDireita, int qtdFrente, int qtdEsquerda);

#endif
