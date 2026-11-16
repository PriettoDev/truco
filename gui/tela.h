#ifndef TELA_H
#define TELA_H

/*
 * tela.h - resolucao logica fixa (1920x1080) ajustada a qualquer janela.
 *
 * Todo o jogo desenha e posiciona em coordenadas logicas, sem saber o
 * tamanho real da janela. A tela cuida do letterbox (barras pretas) e de
 * converter o mouse de volta para coordenadas logicas.
 */
#include "raylib.h"

#define TELA_LARGURA 1920
#define TELA_ALTURA  1080

void tela_iniciar(void);          /* chamar depois do InitWindow */
void tela_encerrar(void);         /* chamar antes do CloseWindow */

/* Tudo que for desenhado entre as duas chamadas usa coordenadas logicas.
   Substituem o BeginDrawing/EndDrawing no laco principal. */
void tela_comecar_desenho(void);
void tela_terminar_desenho(void);

/* Converte um ponto da janela (como o GetMousePosition devolve) para
   coordenadas logicas. Pontos nas barras pretas caem fora de 0..1920/0..1080. */
Vector2 tela_janela_para_logico(Vector2 ponto);
Vector2 tela_mouse(void);         /* mouse ja em coordenadas logicas */

void tela_alternar_tela_cheia(void);

#endif
