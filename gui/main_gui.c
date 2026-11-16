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
#include "cena.h"

/* Tamanho inicial pensado para caber em notebooks; o minimo e metade da
   resolucao logica, abaixo disso as cartas e textos ficam pequenos demais. */
#define JANELA_LARGURA_INICIAL 1280
#define JANELA_ALTURA_INICIAL   720
#define JANELA_LARGURA_MINIMA   960
#define JANELA_ALTURA_MINIMA    540

/* PROVISORIO (some quando a maquina de estados escolher o assento): teclas
   1 a 4 trocam o assento visto, para conferir a rotacao da mesa. */
static int assentoVisto = 0;

static void tratar_teclas_provisorias(void) {
    if (IsKeyPressed(KEY_ONE))   assentoVisto = 0;
    if (IsKeyPressed(KEY_TWO))   assentoVisto = 1;
    if (IsKeyPressed(KEY_THREE)) assentoVisto = 2;
    if (IsKeyPressed(KEY_FOUR))  assentoVisto = 3;
}

/* PROVISORIO: mostra no canto de quem e a camera. Assentos internos (0 a 3),
   os mesmos de Jogador.assento, para comparar direto com a regra p(s+1). */
static void desenhar_assento_visto(void) {
    Font fonte = recurso_fonte(FONTE_TEXTO);
    const char *texto = TextFormat("Vista do assento %d (p%d) \u2014 teclas 1 a 4",
                                   assentoVisto, cena_personagem_do_assento(assentoVisto));
    Vector2 medida = MeasureTextEx(fonte, texto, 40.0f, 1.0f);
    Rectangle fundo = { 24, TELA_ALTURA - medida.y - 48, medida.x + 32, medida.y + 24 };

    DrawRectangleRounded(fundo, 0.4f, 8, Fade(BLACK, 0.7f));
    DrawTextEx(fonte, texto, (Vector2){ fundo.x + 16, fundo.y + 12 }, 40.0f, 1.0f, RAYWHITE);
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
    cena_conferir_posicoes();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) tela_alternar_tela_cheia();
        tratar_teclas_provisorias();

        tela_comecar_desenho();
        cena_desenhar(assentoVisto);
        desenhar_assento_visto();
        tela_terminar_desenho();
    }

    recursos_descarregar();
    tela_encerrar();
    CloseWindow();
    return 0;
}
