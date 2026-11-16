/*
 * main_gui.c - ponto de entrada da interface grafica do Truco no Bar do Pato.
 *
 * Os modulos de src/ continuam donos das regras. Este arquivo so abre a
 * janela e, nas proximas etapas, refaz o fluxo da partida como maquina de
 * estados (o fluxo do main.c le o teclado e por isso nao e reaproveitado).
 */
#include <stdlib.h>   /* srand */
#include <time.h>     /* time */

#include "raylib.h"
#include "tela.h"
#include "recursos.h"

/* Tamanho inicial pensado para caber em notebooks; o minimo e metade da
   resolucao logica, abaixo disso as cartas e textos ficam pequenos demais. */
#define JANELA_LARGURA_INICIAL 1280
#define JANELA_ALTURA_INICIAL   720
#define JANELA_LARGURA_MINIMA   960
#define JANELA_ALTURA_MINIMA    540

/* PROVISORIO (some no commit 4): mostra marcadores, um personagem normal e
   espelhado, as fontes e a mira do mouse para conferir tela e recursos. */
static void desenhar_teste_da_tela(void) {
    Vector2 mouse = tela_mouse();
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    Font naipes = recurso_fonte(FONTE_NAIPES);

    recurso_desenhar(REC_FUNDO_PANORAMA, (Rectangle){ 0, 0, TELA_LARGURA, TELA_ALTURA }, 0, WHITE);
    recurso_desenhar(recurso_personagem(2, VISTA_LADO, EXPRESSAO_NEUTRO),
                     (Rectangle){ 1500, 300, 360, 520 }, 0, WHITE);
    recurso_desenhar(recurso_personagem(2, VISTA_LADO, EXPRESSAO_NEUTRO),
                     (Rectangle){ 60, 300, 360, 520 }, 1, WHITE);       /* espelhado */
    recurso_desenhar(REC_UI_VERSO_CARTA, (Rectangle){ 860, 700, 200, 300 }, 0, WHITE);
    recurso_desenhar(REC_FUNDO_MESA, (Rectangle){ 500, 380, 300, 200 }, 0, WHITE);
    recurso_desenhar(REC_UI_LOGO, (Rectangle){ 1120, 380, 300, 200 }, 0, WHITE);

    DrawTextEx(titulo, "Truco no Bar do Pato", (Vector2){ 40, 30 }, 96, 1, RAYWHITE);
    DrawTextEx(texto, "MÃO de truco: ação, Mãe, João, você, à vista", (Vector2){ 40, 140 }, 48, 1, RAYWHITE);
    DrawTextEx(naipes, "A\u2666  7\u2660  3\u2665  4\u2663", (Vector2){ 1100, 150 }, 96, 1, RAYWHITE);
    DrawTextEx(texto, TextFormat("mouse logico: %.0f, %.0f", mouse.x, mouse.y),
               (Vector2){ 40, 210 }, 36, 1, RAYWHITE);

    DrawLineEx((Vector2){ mouse.x - 30, mouse.y }, (Vector2){ mouse.x + 30, mouse.y }, 4, YELLOW);
    DrawLineEx((Vector2){ mouse.x, mouse.y - 30 }, (Vector2){ mouse.x, mouse.y + 30 }, 4, YELLOW);
}

int main(void) {
    /* O baralho embaralha com rand(); sem semente cada partida seria igual. */
    srand((unsigned)time(NULL));

    /* As flags precisam vir antes do InitWindow: MSAA e HIGHDPI so valem na
       criacao do contexto. HIGHDPI evita a imagem borrada/pixelada em telas
       Retina; VSYNC evita "rasgos" na animacao. */
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI |
                   FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(JANELA_LARGURA_INICIAL, JANELA_ALTURA_INICIAL, "Truco no Bar do Pato");
    SetWindowMinSize(JANELA_LARGURA_MINIMA, JANELA_ALTURA_MINIMA);

    /* Esc vai ser usado para fechar menus; nao pode encerrar a partida. */
    SetExitKey(KEY_NULL);

    tela_iniciar();
    recursos_carregar();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) tela_alternar_tela_cheia();

        tela_comecar_desenho();
        desenhar_teste_da_tela();
        tela_terminar_desenho();
    }

    recursos_descarregar();
    tela_encerrar();
    CloseWindow();
    return 0;
}
