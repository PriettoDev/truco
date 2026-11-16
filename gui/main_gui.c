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

/* Tamanho inicial pensado para caber em notebooks; o minimo e metade da
   resolucao logica, abaixo disso as cartas e textos ficam pequenos demais. */
#define JANELA_LARGURA_INICIAL 1280
#define JANELA_ALTURA_INICIAL   720
#define JANELA_LARGURA_MINIMA   960
#define JANELA_ALTURA_MINIMA    540

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

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Truco no Bar do Pato - janela inicial", 20, 20, 20, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
