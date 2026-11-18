/*
 * main_gui.c - ponto de entrada da interface grafica do Truco no Bar do Pato.
 *
 * Os modulos de src/ continuam donos das regras e partida_gui.c e dono do
 * fluxo (a maquina de estados). Este arquivo so le o mouse e o teclado,
 * avisa a partida e desenha o estado que ela tem.
 */
#include <math.h>     /* sinf, para o pulso do realce */
#include <stdio.h>    /* snprintf */
#include <stdlib.h>   /* srand, atoi */
#include <string.h>   /* strcmp */
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

/* static: a partida nunca pode ser copiada (ver partida_gui.h) e assim vive
   o programa inteiro num lugar so. */
static PartidaGui partida;

static HudDados dados_do_hud(void) {
    HudDados d = { 0 };
    char nome[16];
    int proximo = 0;
    int travaPassou = partida.tempoNoEstado >= PARTIDA_TEMPO_TRAVA;

    d.pontos[0] = partida_pontos(&partida, 0);
    d.pontos[1] = partida_pontos(&partida, 1);
    d.valorMao = partida.mao.aposta.valor;
    for (int i = 0; i < 3; i++) d.resultados[i] = partida.mao.resultados[i];
    d.vira = partida.mesa.vira;
    d.valorPedido = partida.mao.aposta.pedido;   /* 0 sem pedido pendente */

    /* Ativo so quando o modulo aceita o pedido (testado numa copia) e a
       trava passou: o botao acende junto com o momento em que vale clicar. */
    d.resposta = (partida.estado == PARTIDA_RESPOSTA);
    d.pedirAtivo = partida_pode_pedir(&partida, &proximo) && travaPassou;
    d.respostaAtiva = d.resposta && travaPassou;
    d.varAtivo = 0;   /* VAR na Etapa 4 */

    /* Cinza, o rotulo ainda mostra o proximo valor que existe ("PEDIR SEIS"
       quando so a outra dupla pode pedir); no maximo, nao ha o que pedir. */
    if (proximo == 0) proximo = partida_proximo_valor(&partida);
    if (proximo > 0) {
        partida_nome_do_valor(proximo, nome, (int)sizeof nome);
        snprintf(d.rotuloPedir, sizeof d.rotuloPedir, "PEDIR %s", nome);
    } else if (mao_terminou(&partida.mao)) {
        /* Mao encerrada (corrida, fim de mao): o modulo recusa qualquer
           pedido, mas nao porque a mao esta no maximo. */
        snprintf(d.rotuloPedir, sizeof d.rotuloPedir, "PEDIR TRUCO");
    } else {
        snprintf(d.rotuloPedir, sizeof d.rotuloPedir, "NO MÁXIMO");
    }
    return d;
}

/* Cliques e ESPACO vao para a partida, que decide se aceita (travas). Uma
   acao por quadro: o clique que tira a cobertura nao pode jogar uma carta
   no mesmo quadro. */
static void tratar_entrada(float dt) {
    Vector2 mouse = tela_mouse();
    int clique = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    int espaco = IsKeyPressed(KEY_SPACE);
    int vez = (partida.estado == PARTIDA_VEZ_DO_JOGADOR);
    int qtd = partida.mesa.jogadores[partida.assentoDaVez].qtdCartas;
    int sob = vez ? desenho_mao_carta_sob(qtd, mouse) : -1;
    HudDados hud = dados_do_hud();
    HudAcao acao = hud_atualizar(&hud, mouse, dt);

    /* A maozinha aparece sobre o que da para clicar: carta na vez ou botao
       ativo. Na resposta as cartas so se olham. */
    SetMouseCursor((sob >= 0 || hud_mouse_sobre_botao()) ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    switch (partida.estado) {
    case PARTIDA_VEZ_DO_JOGADOR:
        if (acao == HUD_PEDIR_TRUCO) {
            partida_pedir_truco(&partida);
        } else if (clique && sob >= 0 && partida_jogar_carta(&partida, sob)) {
            /* O voo parte do lugar da carta na mao ANTES de ela sair (qtd
               e sob ainda sao de antes da jogada). */
            desenho_voo_iniciar(partida.naMesa[partida.qtdNaMesa - 1].carta, sob, qtd, LADO_EU);
            desenho_mao_retirar(sob, qtd);
        }
        break;
    case PARTIDA_RESPOSTA:
        /* So os botoes respondem: um clique solto ou ESPACO nao pode
           aceitar um truco sem querer. */
        if (acao == HUD_ACEITAR) partida_responder(&partida, RESPOSTA_ACEITAR);
        if (acao == HUD_AUMENTAR) partida_responder(&partida, RESPOSTA_AUMENTAR);
        if (acao == HUD_CORRER) partida_responder(&partida, RESPOSTA_CORRER);
        break;
    default:
        if (clique || espaco) partida_confirmar(&partida);
        break;
    }
}

/* O realce vale da apuracao ate a mesa ser limpa (rodada ou mao nova):
   nas telas de fim, a ultima rodada continua mostrando quem ganhou. A
   partida marca destaque -1 quando nao ha nada para realcar. */
static RealceCarta realce_da_carta(int indice) {
    if (partida.destaque[0] < 0) return REALCE_NENHUM;
    if (indice == partida.destaque[0] || indice == partida.destaque[1]) return REALCE_DESTAQUE;
    return REALCE_APAGADA;
}

/* Na apuracao o realce surge em 0,2 s e depois pulsa devagar, para chamar
   o olho; nas telas de fim fica parado. */
static float intensidade_do_realce(void) {
    float t = partida.tempoNoEstado;
    if (partida.estado != PARTIDA_APURANDO) return 1.0f;
    if (t < 0.2f) return t / 0.2f;
    return 0.8f + 0.2f * cosf((t - 0.2f) * 2.0f * PI * 1.5f);
}

/* Resultado da rodada visto por quem esta na camera: a mesma dupla e "Nos"
   para um e "Eles" para o outro, como no placar. */
static ResultadoVisto resultado_visto(int visto) {
    if (partida.resultadoRodada == EMPATE) return RODADA_EMPATE;
    return (partida.resultadoRodada == visto % 2) ? RODADA_NOS : RODADA_ELES;
}

/* "Seu Bigode e Tonhao": dupla interna d = assentos d e d+2. */
static void nomes_da_dupla(int dupla, char *saida, int tamanho) {
    snprintf(saida, (size_t)tamanho, "%s e %s", partida_nome(dupla), partida_nome(dupla + 2));
}

static void desenhar_mesa(void) {
    int visto = partida_assento_visto(&partida);
    int visivel = partida_mao_visivel(&partida);
    const Jogador *j = partida.mesa.jogadores;
    HudDados hud = dados_do_hud();
    int voando = (partida.estado == PARTIDA_CARTA_VOANDO);
    float intensidade = intensidade_do_realce();

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
        desenho_carta_jogada(partida.naMesa[i].carta, lado, realce_da_carta(i), intensidade);
    }

    /* Camada 5. Privacidade: so quem esta na vez ve a propria mao; nas
       outras telas todos olham, entao a mao aparece como versos. */
    if (visivel >= 0) desenho_mao(j[visivel].mao, j[visivel].qtdCartas);
    else desenho_mao_versos(j[visto].qtdCartas);

    /* Depois da mao: a carta sai de dentro dela e passa por cima. */
    if (voando) desenho_voo();

    hud_desenhar(&hud, visto);   /* camada 6 */
}

static void desenhar_estado(void) {
    int visto = partida_assento_visto(&partida);
    char nomes[96];
    char valor[16];

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
    case PARTIDA_APURANDO:
        desenhar_mesa();
        telas_resultado_rodada(resultado_visto(visto), partida.tempoNoEstado);
        break;
    case PARTIDA_ANUNCIO_PEDIDO:
        desenhar_mesa();
        partida_nome_do_valor(partida.mao.aposta.pedido, valor, (int)sizeof valor);
        telas_anuncio_pedido(partida_nome(partida.assentoPediu), valor, partida.tempoNoEstado);
        break;
    case PARTIDA_COBERTURA_RESPOSTA:
        desenhar_mesa();
        partida_nome_do_valor(partida.mao.aposta.pedido, valor, (int)sizeof valor);
        telas_cobertura_resposta(visto, partida_nome(visto), partida_nome(partida.assentoPediu), valor,
                                 partida.tempoNoEstado, PARTIDA_TEMPO_TRAVA);
        break;
    case PARTIDA_RESPOSTA:
        desenhar_mesa();
        partida_nome_do_valor(partida.mao.aposta.pedido, valor, (int)sizeof valor);
        telas_pedido_pendente(partida_nome(partida.assentoPediu), valor);
        break;
    case PARTIDA_FIM_MAO:
        desenhar_mesa();
        if (partida.duplaVencedoraMao == 0 || partida.duplaVencedoraMao == 1) {
            char correu[96];
            nomes_da_dupla(partida.duplaVencedoraMao, nomes, (int)sizeof nomes);
            if (partida.assentoCorreu >= 0) nomes_da_dupla(partida.assentoCorreu % 2, correu, (int)sizeof correu);
            telas_fim_mao(nomes, partida.assentoCorreu >= 0 ? correu : NULL, partida.pontosDaMao,
                          partida.tempoNoEstado, PARTIDA_TEMPO_TRAVA);
        } else {
            telas_fim_mao(NULL, NULL, 0, partida.tempoNoEstado, PARTIDA_TEMPO_TRAVA);
        }
        break;
    case PARTIDA_FIM_PARTIDA: {
        int d = partida.duplaVencedoraPartida;
        nomes_da_dupla(d, nomes, (int)sizeof nomes);
        telas_fim_partida(d, d + 2, nomes, partida_pontos(&partida, d), partida_pontos(&partida, 1 - d),
                          partida.tempoNoEstado, PARTIDA_TEMPO_TRAVA);
        break;
    }
    default:
        desenhar_mesa();
        break;
    }
}

/* PROVISORIO: atalho de teste para chegar logo ao fim de partida.
   "make -C gui run ARGS='--placar 11 11'" comeca com 11 x 11 (dupla de
   Seu Bigode x dupla de Dona Marlene). Tirar antes da entrega. */
static void placar_de_teste(int argc, char **argv) {
    for (int i = 1; i + 2 < argc; i++) {   /* --placar precisa de mais dois */
        if (strcmp(argv[i], "--placar") == 0) {
            int a = atoi(argv[i + 1]), b = atoi(argv[i + 2]);
            /* Abaixo de 12: com 12 a partida ja teria acabado. */
            if (a < 0 || b < 0 || a >= PONTOS_PARA_VENCER || b >= PONTOS_PARA_VENCER) {
                TraceLog(LOG_WARNING, "PROVISORIO: --placar precisa de dois numeros de 0 a %d",
                         PONTOS_PARA_VENCER - 1);
                return;
            }
            partida.mesa.pontosDupla1 = a;   /* dupla interna 0 */
            partida.mesa.pontosDupla2 = b;   /* dupla interna 1 */
            TraceLog(LOG_INFO, "PROVISORIO: partida comeca com %d x %d", a, b);
            return;
        }
    }
}

int main(int argc, char **argv) {
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
    placar_de_teste(argc, argv);   /* PROVISORIO */

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        int qtdVez;

        if (IsKeyPressed(KEY_F11)) tela_alternar_tela_cheia();
        tratar_entrada(dt);
        partida_atualizar(&partida, dt);

        /* O destaque da mao so anda na vez; fora dela volta tudo ao lugar
           (quantidade 0 zera), para o proximo jogador nao herdar uma carta
           levantada. */
        qtdVez = partida.mesa.jogadores[partida.assentoDaVez].qtdCartas;
        desenho_mao_atualizar(partida.estado == PARTIDA_VEZ_DO_JOGADOR ? qtdVez : 0, tela_mouse(), dt);
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
