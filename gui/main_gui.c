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

/* Tamanho inicial pensado para caber em notebooks; o minimo e metade da
   resolucao logica, abaixo disso as cartas e textos ficam pequenos demais. */
#define JANELA_LARGURA_INICIAL 1280
#define JANELA_ALTURA_INICIAL   720
#define JANELA_LARGURA_MINIMA   960
#define JANELA_ALTURA_MINIMA    540

/* PROVISORIO (some no commit 4): grade e mira do mouse para conferir o
   letterbox e a conversao do mouse redimensionando a janela. */
static void desenhar_teste_da_tela(void) {
    Vector2 mouse = tela_mouse();

    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, (Color){ 20, 60, 40, 255 });
    for (int x = 0; x <= TELA_LARGURA; x += 120)
        DrawLine(x, 0, x, TELA_ALTURA, (Color){ 40, 100, 70, 255 });
    for (int y = 0; y <= TELA_ALTURA; y += 120)
        DrawLine(0, y, TELA_LARGURA, y, (Color){ 40, 100, 70, 255 });
    DrawRectangleLinesEx((Rectangle){ 0, 0, TELA_LARGURA, TELA_ALTURA }, 8, GOLD);
    DrawCircle(TELA_LARGURA / 2, TELA_ALTURA / 2, 200, (Color){ 200, 60, 60, 255 });
    DrawText("1920 x 1080 logico - F11 tela cheia", 40, 40, 40, RAYWHITE);
    DrawText(TextFormat("janela %d x %d pontos | %d x %d pixels",
                        GetScreenWidth(), GetScreenHeight(),
                        GetRenderWidth(), GetRenderHeight()), 40, 100, 30, RAYWHITE);
    DrawText(TextFormat("mouse logico: %.0f, %.0f", mouse.x, mouse.y), 40, 150, 30, RAYWHITE);

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

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) tela_alternar_tela_cheia();

        tela_comecar_desenho();
        desenhar_teste_da_tela();
        tela_terminar_desenho();
    }

    tela_encerrar();
    CloseWindow();
    return 0;
}
