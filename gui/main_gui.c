/*
 * main_gui.c - ponto de entrada da interface grafica do Truco no Bar do Pato.
 *
 * Os modulos de src/ continuam donos das regras e partida_gui.c e dono do
 * fluxo (a maquina de estados). Este arquivo so le o mouse e o teclado,
 * avisa a partida e desenha o estado que ela tem.
 */
#include <stdlib.h>   /* srand */
#include <time.h>     /* time */

#include "raylib.h"
#include "tela.h"
#include "recursos.h"
#include "cena.h"
#include "desenho_cartas.h"
#include "hud.h"
#include "conferencia.h"
#include "partida_gui.h"
#include "telas_partida.h"

/* Tamanho inicial pensado para caber em notebooks; o minimo e metade da
   resolucao logica, abaixo disso as cartas e textos ficam pequenos demais. */
#define JANELA_LARGURA_INICIAL 1280
#define JANELA_ALTURA_INICIAL   720
#define JANELA_LARGURA_MINIMA   960
#define JANELA_ALTURA_MINIMA    540

/* Truco e VAR ainda nao existem nesta etapa: os botoes aparecem cinza. */
#define BOTOES_ATIVOS 0

/* static: a partida nunca pode ser copiada (ver partida_gui.h) e assim vive
   o programa inteiro num lugar so. */
static PartidaGui partida;

/* Cliques e ESPACO vao para a partida, que decide se aceita (travas). Uma
   acao por quadro: o clique que tira a cobertura nao pode jogar uma carta
   no mesmo quadro. */
static void tratar_entrada(void) {
    Vector2 mouse = tela_mouse();
    int clique = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    int espaco = IsKeyPressed(KEY_SPACE);
    int vez = (partida.estado == PARTIDA_VEZ_DO_JOGADOR);
    int qtd = partida.mesa.jogadores[partida.assentoDaVez].qtdCartas;
    int sob = vez ? desenho_mao_carta_sob(qtd, mouse) : -1;

    /* Na vez, a mao do jogador e o que se clica; a maozinha mostra isso. */
    SetMouseCursor(sob >= 0 ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    if (vez) {
        /* O voo parte do lugar da carta na mao ANTES de ela sair (qtd e sob
           ainda sao de antes da jogada). */
        if (clique && sob >= 0 && partida_jogar_carta(&partida, sob)) {
            desenho_voo_iniciar(partida.naMesa[partida.qtdNaMesa - 1].carta, sob, qtd, LADO_EU);
            desenho_mao_retirar(sob, qtd);
        }
    } else if (clique || espaco) {
        partida_confirmar(&partida);
    }
}

static HudDados dados_do_hud(void) {
    HudDados d;
    d.pontos[0] = partida_pontos(&partida, 0);
    d.pontos[1] = partida_pontos(&partida, 1);
    d.valorMao = partida.mao.aposta.valor;
    for (int i = 0; i < 3; i++) d.resultados[i] = partida.mao.resultados[i];
    d.vira = partida.mesa.vira;
    return d;
}

static void desenhar_mesa(void) {
    int visto = partida_assento_visto(&partida);
    int visivel = partida_mao_visivel(&partida);
    const Jogador *j = partida.mesa.jogadores;
    HudDados hud = dados_do_hud();
    int voando = (partida.estado == PARTIDA_CARTA_VOANDO);

    cena_desenhar(visto);                        /* camadas 1 a 4 */
    desenho_versos_dos_outros(j[cena_assento_na_posicao(visto, POSICAO_DIREITA)].qtdCartas,
                              j[cena_assento_na_posicao(visto, POSICAO_FRENTE)].qtdCartas,
                              j[cena_assento_na_posicao(visto, POSICAO_ESQUERDA)].qtdCartas);
    desenho_monte();
    desenho_vira(partida.mesa.vira);

    /* Cada carta fica do lado de quem a jogou, visto por quem esta na vez:
       quando a camera troca de jogador, a cruz gira junto. Durante o voo a
       ultima carta ainda nao chegou: ela e desenhada pelo voo, por cima. */
    for (int i = 0; i < partida.qtdNaMesa - (voando ? 1 : 0); i++) {
        LadoDaMesa lado = (LadoDaMesa)((partida.naMesa[i].assento - visto + 4) % 4);
        desenho_carta_jogada(partida.naMesa[i].carta, lado);
    }

    /* Camada 5. Privacidade: so quem esta na vez ve a propria mao; nas
       outras telas todos olham, entao a mao aparece como versos. */
    if (visivel >= 0) desenho_mao(j[visivel].mao, j[visivel].qtdCartas);
    else desenho_mao_versos(j[visto].qtdCartas);

    /* Depois da mao: a carta sai de dentro dela e passa por cima. */
    if (voando) desenho_voo();

    hud_desenhar(&hud, visto, BOTOES_ATIVOS);   /* camada 6 */
}

static void desenhar_estado(void) {
    int visto = partida_assento_visto(&partida);

    switch (partida.estado) {
    case PARTIDA_INICIO_MAO:
        /* Dura um quadro so (distribui e ja vai para a cobertura); a tela
           fica coberta para a mao nova nunca aparecer descoberta. */
        DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, BLACK);
        break;
    case PARTIDA_COBERTURA:
        desenhar_mesa();
        telas_cobertura(visto, partida_nome(visto), partida.tempoNoEstado, PARTIDA_TEMPO_TRAVA);
        break;
    case PARTIDA_FIM_MAO:
        desenhar_mesa();
        telas_aviso_provisorio("Fim da mão", "clique ou ESPAÇO para continuar");
        break;
    case PARTIDA_FIM_PARTIDA:
        desenhar_mesa();
        telas_aviso_provisorio("Fim da partida", "clique para jogar de novo");
        break;
    default:
        desenhar_mesa();
        break;
    }
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
    conferir_posicoes();
    partida_iniciar(&partida);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        int qtdVez;

        if (IsKeyPressed(KEY_F11)) tela_alternar_tela_cheia();
        tratar_entrada();
        partida_atualizar(&partida, dt);

        /* O destaque da mao so anda na vez; fora dela volta tudo ao lugar
           (quantidade 0 zera), para o proximo jogador nao herdar uma carta
           levantada. */
        qtdVez = partida.mesa.jogadores[partida.assentoDaVez].qtdCartas;
        desenho_mao_atualizar(partida.estado == PARTIDA_VEZ_DO_JOGADOR ? qtdVez : 0, tela_mouse(), dt);
        hud_atualizar(tela_mouse(), dt, BOTOES_ATIVOS);
        desenho_voo_atualizar(dt);
        /* A mao na tela e sempre a de quem a camera mostra (de frente ou
           em versos): as que sobraram deslizam durante o voo. */
        desenho_mao_deslizar(partida.mesa.jogadores[partida_assento_visto(&partida)].qtdCartas, dt);

        tela_comecar_desenho();
        desenhar_estado();
        tela_terminar_desenho();
    }

    recursos_descarregar();
    tela_encerrar();
    CloseWindow();
    return 0;
}
