#include <math.h>   /* expf */
#include <stdio.h>  /* snprintf */

#include "hud.h"
#include "rodada.h"          /* EMPATE e INDEFINIDO */
#include "desenho_cartas.h"
#include "recursos.h"

#define HUD_SUAVIDADE 16.0f   /* rapidez do destaque dos botoes */

static const Color COR_PAINEL  = {  18,  14,  12, 210 };
static const Color COR_MOLDURA = { 214, 170,  80, 255 };
static const Color COR_TEXTO   = { 245, 236, 214, 255 };
static const Color COR_DESATIVADO = { 92, 92, 92, 255 };

/* Destaque atual de cada botao (0 = normal, 1 = mouse em cima), na ordem
   de HudAcao: o indice e a propria acao. */
#define QTD_BOTOES 6
static float destaque[QTD_BOTOES];
static int sobreBotao;

static void painel(Rectangle r) {
    DrawRectangleRounded(r, 0.18f, 10, COR_PAINEL);
    DrawRectangleRoundedLinesEx(r, 0.18f, 10, 3.0f, COR_MOLDURA);
}

/* Texto centralizado horizontalmente em cx, com o topo em y. */
static void texto_centro(Font fonte, const char *texto, float cx, float y, float tamanho, Color cor) {
    Vector2 medida = MeasureTextEx(fonte, texto, tamanho, 1.0f);
    DrawTextEx(fonte, texto, (Vector2){ cx - medida.x * 0.5f, y }, tamanho, 1.0f, cor);
}

/* ---------------------------------------------------------------- placar */

/* Cor da bolinha do ponto de vista de quem joga: verde se a NOSSA dupla
   fez a rodada, vermelho se foi a deles. */
static void bolinha_rodada(Vector2 c, int resultado, int duplaNos) {
    const float raio = 11.0f;
    if (resultado == INDEFINIDO) {
        DrawCircleLinesV(c, raio, Fade(COR_TEXTO, 0.6f));   /* ainda nao jogada */
    } else if (resultado == EMPATE) {
        DrawCircleV(c, raio, HUD_COR_EMPATE);
    } else {
        DrawCircleV(c, raio, (resultado == duplaNos) ? HUD_COR_NOS : HUD_COR_ELES);
    }
}

/* "VALE 1 -> 3?" enquanto o pedido nao foi respondido. A seta vem da fonte
   de naipes: as de titulo e de texto nao tem U+2192 (conferir_fontes.py),
   entao o trecho e desenhado em tres pedacos. O chip fica mais largo e
   claro, para todo mundo ver que a mao esta em negociacao. */
#define SETA "\xE2\x86\x92"   /* U+2192 em UTF-8, igual em qualquer compilador */

static void desenhar_vale_pedido(const HudDados *d, float meio, Rectangle chip) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    Font naipes = recurso_fonte(FONTE_NAIPES);
    const float tam = 40.0f, tamSeta = 34.0f, folga = 6.0f;
    const char *antes = TextFormat("%d", d->valorMao);
    char depois[16];
    Vector2 mAntes, mSeta, mDepois;
    float x, y = chip.y + 32.0f;

    snprintf(depois, sizeof depois, "%d?", d->valorPedido);
    mAntes = MeasureTextEx(titulo, antes, tam, 1.0f);
    mSeta = MeasureTextEx(naipes, SETA, tamSeta, 1.0f);
    mDepois = MeasureTextEx(titulo, depois, tam, 1.0f);

    chip.width = 156.0f;
    chip.x = meio - chip.width * 0.5f;
    DrawRectangleRounded(chip, 0.3f, 8, ColorBrightness(COR_MOLDURA, 0.25f));
    DrawRectangleRoundedLinesEx(chip, 0.3f, 8, 3.0f, COR_TEXTO);
    texto_centro(texto, "VALE", meio, chip.y + 4.0f, 22.0f, COR_PAINEL);

    x = meio - (mAntes.x + folga + mSeta.x + folga + mDepois.x) * 0.5f;
    DrawTextEx(titulo, antes, (Vector2){ x, y }, tam, 1.0f, COR_PAINEL);
    x += mAntes.x + folga;
    DrawTextEx(naipes, SETA, (Vector2){ x, y + (tam - mSeta.y) * 0.5f }, tamSeta, 1.0f, COR_PAINEL);
    x += mSeta.x + folga;
    DrawTextEx(titulo, depois, (Vector2){ x, y }, tam, 1.0f, COR_PAINEL);
}

static void desenhar_placar(const HudDados *d, int assentoVisto) {
    Rectangle r = HUD_PLACAR;
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    int duplaNos = assentoVisto % 2;          /* numeracao interna 0/1 */
    float colNos = r.x + 82.0f, colEles = r.x + r.width - 82.0f, meio = r.x + r.width * 0.5f;
    Rectangle chip = { meio - 54.0f, r.y + 14.0f, 108.0f, 84.0f };

    painel(r);
    texto_centro(texto, "NÓS", colNos, r.y + 12.0f, 28.0f, HUD_COR_NOS);
    texto_centro(titulo, TextFormat("%d", d->pontos[duplaNos]), colNos, r.y + 42.0f, 76.0f, COR_TEXTO);
    texto_centro(texto, "ELES", colEles, r.y + 12.0f, 28.0f, HUD_COR_ELES);
    texto_centro(titulo, TextFormat("%d", d->pontos[1 - duplaNos]), colEles, r.y + 42.0f, 76.0f, COR_TEXTO);

    /* Valor da mao no meio, em destaque: e o que muda com o truco. */
    if (d->valorPedido > 0) {
        desenhar_vale_pedido(d, meio, chip);
    } else {
        DrawRectangleRounded(chip, 0.3f, 8, COR_MOLDURA);
        texto_centro(texto, "VALE", meio, chip.y + 4.0f, 22.0f, COR_PAINEL);
        texto_centro(titulo, TextFormat("%d", d->valorMao), meio, chip.y + 24.0f, 58.0f, COR_PAINEL);
    }

    for (int i = 0; i < 3; i++) {
        bolinha_rodada((Vector2){ meio + (i - 1) * 34.0f, r.y + r.height - 26.0f },
                       d->resultados[i], duplaNos);
    }
}

/* ------------------------------------------------- cartas mais fortes */

/* A ordem vem de carta_forca do modulo carta, e nao de uma tabela aqui: se
   a regra de desempate entre naipes mudar, o painel acompanha sozinho. */
static void manilhas_em_ordem(Carta vira, Carta saida[4]) {
    Valor manilha = manilha_da_vira(vira);

    for (int n = 0; n < NUM_NAIPES; n++) {
        saida[n].naipe = (Naipe)n;
        saida[n].valor = manilha;
    }
    /* Ordenacao por insercao, da mais forte para a mais fraca (sao so 4). */
    for (int i = 1; i < 4; i++) {
        Carta atual = saida[i];
        int j = i - 1;
        while (j >= 0 && carta_forca(saida[j], manilha) < carta_forca(atual, manilha)) {
            saida[j + 1] = saida[j];
            j--;
        }
        saida[j + 1] = atual;
    }
}

static void desenhar_cartas_fortes(const HudDados *d) {
    Rectangle r = HUD_CARTAS_FORTES;
    Font texto = recurso_fonte(FONTE_TEXTO);
    const float largura = 62.0f, altura = largura * DESENHO_CARTA_PROPORCAO, passo = 74.0f;
    float y = r.y + 64.0f;
    float x = r.x + 20.0f;
    Carta manilhas[4];

    painel(r);
    texto_centro(texto, "CARTAS MAIS FORTES", r.x + r.width * 0.5f, r.y + 12.0f, 30.0f, COR_MOLDURA);

    desenho_carta_face(d->vira, (Rectangle){ x, y, largura, altura });
    texto_centro(texto, "vira", x + largura * 0.5f, y + altura + 8.0f, 22.0f, COR_TEXTO);

    /* Separador entre a vira e as manilhas: a vira nao e uma delas. */
    DrawLineEx((Vector2){ x + largura + 16.0f, y + 6.0f },
               (Vector2){ x + largura + 16.0f, y + altura - 6.0f }, 2.0f, Fade(COR_MOLDURA, 0.7f));

    manilhas_em_ordem(d->vira, manilhas);
    x += largura + 34.0f;
    for (int i = 0; i < 4; i++) {
        desenho_carta_face(manilhas[i], (Rectangle){ x + i * passo, y, largura, altura });
    }
    /* So caracteres carregados em recursos.c: uma seta viraria "?". */
    texto_centro(texto, "da mais forte à mais fraca", x + (3 * passo + largura) * 0.5f,
                 y + altura + 8.0f, 22.0f, COR_TEXTO);
}

/* ---------------------------------------------------------------- botoes */

static void desenhar_botao(Rectangle r, const char *rotulo, Color cor, float destaque, float tamanhoTexto,
                           int ativo) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    /* Destaque: cresce ate 5% a partir do centro e clareia a cor. */
    float escala = 1.0f + 0.05f * destaque;
    Rectangle e = { r.x + r.width * (1.0f - escala) * 0.5f, r.y + r.height * (1.0f - escala) * 0.5f,
                    r.width * escala, r.height * escala };
    Color fundo = ColorBrightness(cor, 0.25f * destaque);
    /* Desativado: cinza e texto apagado, para ninguem achar que travou. */
    Color corTexto = ativo ? COR_TEXTO : Fade(COR_TEXTO, 0.45f);
    if (!ativo) fundo = COR_DESATIVADO;

    DrawRectangleRounded((Rectangle){ e.x + 4, e.y + 6, e.width, e.height }, 0.35f, 10, Fade(BLACK, 0.4f));
    DrawRectangleRounded(e, 0.35f, 10, fundo);
    DrawRectangleRoundedLinesEx(e, 0.35f, 10, 3.0f, Fade(corTexto, 0.5f + 0.5f * destaque));
    texto_centro(titulo, rotulo, e.x + e.width * 0.5f,
                 e.y + (e.height - tamanhoTexto) * 0.5f, tamanhoTexto * escala, corTexto);
}

/* Os botoes de cada momento: na vez, PEDIR TRUCO e VAR; na resposta,
   ACEITAR, PEDIR MAIS (so se o modulo aceitar), CORRER e VAR. */
typedef struct {
    HudAcao acao;
    Rectangle area;
    const char *rotulo;
    Color cor;
    float tamanhoTexto;
    int ativo;
} Botao;

static int montar_botoes(const HudDados *d, Botao b[QTD_BOTOES]) {
    int n = 0;
    if (d->resposta) {
        b[n++] = (Botao){ HUD_ACEITAR, HUD_BOTAO_ACEITAR, "ACEITAR", { 46, 130, 70, 255 }, 44.0f, d->respostaAtiva };
        if (d->pedirAtivo)
            b[n++] = (Botao){ HUD_AUMENTAR, HUD_BOTAO_AUMENTAR, d->rotuloPedir, { 190, 60, 30, 255 }, 40.0f,
                              d->respostaAtiva };
        b[n++] = (Botao){ HUD_CORRER, HUD_BOTAO_CORRER, "CORRER", { 110, 84, 70, 255 }, 44.0f, d->respostaAtiva };
    } else {
        b[n++] = (Botao){ HUD_PEDIR_TRUCO, HUD_BOTAO_TRUCO, d->rotuloPedir, { 190, 60, 30, 255 }, 40.0f,
                          d->pedirAtivo };
    }
    b[n++] = (Botao){ HUD_VAR, HUD_BOTAO_VAR, "VAR", { 40, 70, 120, 255 }, 38.0f, d->varAtivo };
    return n;
}

HudAcao hud_atualizar(const HudDados *dados, Vector2 mouse, float dt) {
    float passo = 1.0f - expf(-HUD_SUAVIDADE * dt);   /* igual em qualquer FPS */
    Botao b[QTD_BOTOES];
    int n = montar_botoes(dados, b);
    HudAcao clicado = HUD_NENHUMA;
    float alvo[QTD_BOTOES] = { 0 };

    sobreBotao = 0;
    for (int i = 0; i < n; i++) {
        /* Desativado nao destaca nem recebe clique: ninguem acha que travou. */
        if (!b[i].ativo || !CheckCollisionPointRec(mouse, b[i].area)) continue;
        alvo[b[i].acao] = 1.0f;
        sobreBotao = 1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) clicado = b[i].acao;
    }
    for (int i = 0; i < QTD_BOTOES; i++) destaque[i] += (alvo[i] - destaque[i]) * passo;
    return clicado;
}

int hud_mouse_sobre_botao(void) {
    return sobreBotao;
}

void hud_desenhar(const HudDados *dados, int assentoVisto) {
    Botao b[QTD_BOTOES];
    int n = montar_botoes(dados, b);

    desenhar_placar(dados, assentoVisto);
    desenhar_cartas_fortes(dados);
    for (int i = 0; i < n; i++) {
        desenhar_botao(b[i].area, b[i].rotulo, b[i].cor, destaque[b[i].acao], b[i].tamanhoTexto, b[i].ativo);
    }
}
