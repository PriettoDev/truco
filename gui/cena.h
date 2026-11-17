#ifndef CENA_H
#define CENA_H

/*
 * cena.h - camadas do bar desenhadas atras da interface, de tras para a
 * frente: panorama, paineis do bar, personagens e mesa.
 *
 * Todas as posicoes estao aqui como constantes em coordenadas logicas
 * (1920x1080) para serem ajustadas quando a arte chegar, sem caçar numeros
 * dentro das funcoes.
 */
#include "raylib.h"

/* A arte "lado" dos personagens e desenhada olhando para a DIREITA. Assim o
   personagem da ESQUERDA da tela ja olha para a mesa e e usado como esta; o
   da DIREITA precisa ser espelhado. Se a arte vier virada para a esquerda,
   troque para 0 e o espelhamento inverte nos dois lados. */
#define CENA_ARTE_LADO_OLHA_PARA_DIREITA 1

/* Proporcao (largura/altura) assumida para o panorama enquanto o arquivo nao
   existe: so para o marcador provisorio ja ocupar a area real com margem. */
#define CENA_PANORAMA_PROPORCAO_PROVISORIA 2.4f

/* Paineis: areas sobre objetos ja pintados no panorama. Medidas com o
   panorama centralizado; quando houver paralaxe, andam junto com ele.
   REGRA: os quatro serao clicaveis, entao nenhum pode ficar atras de
   personagem nem da mesa. conferir_posicoes() (conferencia.c) avisa no
   terminal se algum ajuste quebrar isso. Posicoes provisorias: as finais
   vem com a arte. */
#define CENA_PAINEL_LOUSA      ((Rectangle){  60,  50, 380, 230 })   /* placar  */
#define CENA_PAINEL_MURAL      ((Rectangle){ 470,  80, 260, 190 })   /* ranking */
#define CENA_PAINEL_TV_VAR     ((Rectangle){1160,  60, 290, 190 })   /* VAR     */
#define CENA_PAINEL_CADERNETA  ((Rectangle){1180, 330, 160, 120 })   /* entre o parceiro
                                                                        e o da direita */

/* Personagens: caixa onde a imagem cabe, apoiada embaixo e centralizada.
   As caixas descem atras da mesa de proposito: a mesa esconde a cintura. */
#define CENA_PERSONAGEM_ESQUERDA ((Rectangle){  40, 300, 520, 760 })
#define CENA_PERSONAGEM_FRENTE   ((Rectangle){ 760, 170, 400, 580 })
#define CENA_PERSONAGEM_DIREITA  ((Rectangle){1360, 300, 520, 760 })

/* Rostos: parte de cima de cada caixa, onde a cabeca deve ficar. Nada da
   interface pode cobrir essas areas (a conferencia do inicio verifica).
   PROVISORIO: ajustar quando a arte dos personagens chegar. */
#define CENA_ROSTO_ESQUERDA ((Rectangle){  40, 300, 520, 230 })
#define CENA_ROSTO_FRENTE   ((Rectangle){ 760, 170, 400, 180 })
#define CENA_ROSTO_DIREITA  ((Rectangle){1360, 300, 520, 230 })

/* Mesa: tampo ocupando a parte de baixo; o centro recebe a vira e as cartas
   jogadas, e a mao do jogador fica por cima, na borda de baixo. */
#define CENA_MESA ((Rectangle){ 160, 600, 1600, 480 })

/* Lugares dos outros tres jogadores na tela de quem esta jogando. */
typedef enum { POSICAO_DIREITA, POSICAO_FRENTE, POSICAO_ESQUERDA } PosicaoNaTela;

/* Jogo no sentido anti-horario. Vista do assento A:
   direita = (A+1)%4, frente = (A+2)%4 (parceiro), esquerda = (A+3)%4. */
int cena_assento_na_posicao(int assentoVisto, PosicaoNaTela posicao);

/* O jogador do assento s (0 a 3) usa o personagem p(s+1). */
int cena_personagem_do_assento(int assento);

/* Desenha as quatro camadas do fundo na vista do jogador assentoVisto. */
void cena_desenhar(int assentoVisto);

#endif
