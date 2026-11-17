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
#define DESENHO_MAO_TOPO          840.0f   /* baixa o bastante para a cruz caber */
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
#define DESENHO_VERSOS_FRENTE_CENTRO   ((Vector2){  960, 530 })   /* acima da cruz */
#define DESENHO_VERSOS_FRENTE_GRAUS    180.0f
#define DESENHO_VERSOS_ESQUERDA_CENTRO ((Vector2){  330, 790 })
#define DESENHO_VERSOS_ESQUERDA_GRAUS   90.0f
#define DESENHO_VERSOS_DIREITA_CENTRO  ((Vector2){ 1590, 790 })
#define DESENHO_VERSOS_DIREITA_GRAUS   -90.0f

/* Cartas jogadas: uma cruz no meio da mesa, cada carta na frente de quem a
   jogou (embaixo = quem esta vendo). O tamanho e o maior que cabe entre os
   versos do parceiro (embaixo em y 596) e a mao em repouso (y 840): quando
   a sua carta esta na mesa, a sua mao nunca esta levantada. Os lados ficam
   afastados o bastante para nao cobrir o canto das outras, e por isso a
   cruz fica um pouco a esquerda: assim nao encosta na vira (x 1100). */
#define DESENHO_JOGADA_CENTRO        ((Vector2){ 940, 722 })
#define DESENHO_JOGADA_LARGURA       76.0f
#define DESENHO_JOGADA_AFASTAMENTO_V 59.0f    /* do centro ate a carta de baixo/de cima */
#define DESENHO_JOGADA_AFASTAMENTO_H 100.0f   /* dos lados: sem cobrir o canto das outras */

/* Voo da carta da mao ate a mesa. Aproximacao exponencial pelo tempo, igual
   a da mao: com 12, depois de 0,35 s (PARTIDA_TEMPO_VOO) falta menos de 2%
   do caminho, e o resto some quando a carta assenta na cruz. */
#define DESENHO_VOO_SUAVIDADE 12.0f

/* Lado da mesa em relacao a quem esta vendo: (assento - assentoVisto + 4) % 4. */
typedef enum { LADO_EU, LADO_DIREITA, LADO_FRENTE, LADO_ESQUERDA } LadoDaMesa;

/* Face de uma carta no retangulo dado (sem rotacao). */
void desenho_carta_face(Carta carta, Rectangle area);

/* Face girada em torno do centro (graus no sentido horario, como na raylib). */
void desenho_carta_face_girada(Carta carta, Vector2 centro, float largura, float graus);

/* Realce na apuracao: a carta que ganhou (no empate, a melhor de cada
   dupla) ganha moldura dourada e as outras escurecem, para todos verem de
   longe quem fez a rodada. */
typedef enum { REALCE_NENHUM, REALCE_DESTAQUE, REALCE_APAGADA } RealceCarta;

/* Carta jogada, no lugar da cruz que corresponde ao lado de quem jogou.
   intensidade (0 a 1) controla o realce, para ele aparecer aos poucos e
   pulsar; com REALCE_NENHUM e ignorada. */
void desenho_carta_jogada(Carta carta, LadoDaMesa lado, RealceCarta realce, float intensidade);
Rectangle desenho_area_cartas_jogadas(void);

/* Voo: comeca no lugar exato da carta na mao (levantada ou nao) e vai ate
   o lugar dela na cruz. So desenha; quem decide quanto dura e a partida. */
void desenho_voo_iniciar(Carta carta, int posicaoNaMao, int quantidadeNaMao, LadoDaMesa destino);
void desenho_voo_atualizar(float dt);
void desenho_voo(void);

/* Mao do jogador: atualizar uma vez por quadro (anima a carta sob o mouse)
   e depois desenhar. mouse em coordenadas logicas, dt em segundos. */
void desenho_mao_atualizar(int quantidade, Vector2 mouse, float dt);
void desenho_mao(const Carta cartas[], int quantidade);

/* Uma carta saiu da mao (chamar logo depois de desenho_voo_iniciar, que
   ainda usa o lugar antigo dela). As outras guardam onde estavam. */
void desenho_mao_retirar(int posicao, int quantidadeAntes);

/* Uma vez por quadro, com a quantidade de cartas da mao que esta na tela:
   as cartas restantes deslizam ate o lugar centralizado. Se a quantidade
   mudou por outro motivo (mao nova, outro jogador), vao direto ao lugar. */
void desenho_mao_deslizar(int quantidade, float dt);

/* Mesma posicao da mao, mas so os versos: quando todos olham a tela
   (apuracao e telas de fim), ninguem pode ver as cartas de ninguem. */
void desenho_mao_versos(int quantidade);

/* Indice da carta da mao sob o mouse (mesma area do destaque), ou -1. */
int desenho_mao_carta_sob(int quantidade, Vector2 mouse);

void desenho_vira(Carta vira);
void desenho_monte(void);

/* Areas que a interface nunca pode cobrir (usadas na conferencia do inicio):
   a mao inteira, incluindo a altura da carta levantada, e vira + monte. */
Rectangle desenho_area_mao(int quantidade);
Rectangle desenho_area_vira_e_monte(void);
Rectangle desenho_area_versos_frente(void);   /* leque do parceiro, inteiro */

/* Versos diante de cada um dos outros 3 jogadores (0 a 3 cartas cada). */
void desenho_versos_dos_outros(int qtdDireita, int qtdFrente, int qtdEsquerda);

#endif
