#include <stddef.h>   /* NULL: mao empatada nao tem dupla vencedora */

#include "telas_partida.h"
#include "tela.h"
#include "recursos.h"
#include "cena.h"
#include "hud.h"   /* cores de Nos/Eles/Empate, iguais as do placar */

static const Color COR_COBERTURA = {  18,  14,  12, 246 };   /* quase opaca: nada da mesa vaza */
static const Color COR_PAINEL    = {  18,  14,  12, 225 };
static const Color COR_MOLDURA   = { 214, 170,  80, 255 };
static const Color COR_TEXTO     = { 245, 236, 214, 255 };

/* Caixa do personagem na cobertura: no meio, acima do nome. */
#define COBERTURA_PERSONAGEM ((Rectangle){ 760, 120, 400, 520 })

/* Fim de partida: os dois vencedores lado a lado, acima do texto. */
#define FIM_PERSONAGEM_A ((Rectangle){ 520, 80, 400, 470 })
#define FIM_PERSONAGEM_B ((Rectangle){ 1000, 80, 400, 470 })

#define TEMPO_SURGIR 0.2f   /* plaquinha da rodada aparece aos poucos */

static void texto_em(Font fonte, const char *texto, float cx, float y, float tamanho, Color cor) {
    Vector2 medida = MeasureTextEx(fonte, texto, tamanho, 1.0f);
    DrawTextEx(fonte, texto, (Vector2){ cx - medida.x * 0.5f, y }, tamanho, 1.0f, cor);
}

static void texto_centro(Font fonte, const char *texto, float y, float tamanho, Color cor) {
    texto_em(fonte, texto, TELA_LARGURA * 0.5f, y, tamanho, cor);
}

/* Encaixa a imagem na caixa sem deformar, apoiada embaixo (como na cena). */
static Rectangle caber(Rectangle caixa, Vector2 tamanho) {
    float escala;
    if (tamanho.x <= 0 || tamanho.y <= 0) return caixa;
    escala = caixa.width / tamanho.x;
    if (tamanho.y * escala > caixa.height) escala = caixa.height / tamanho.y;
    return (Rectangle){ caixa.x + (caixa.width - tamanho.x * escala) * 0.5f,
                        caixa.y + caixa.height - tamanho.y * escala,
                        tamanho.x * escala, tamanho.y * escala };
}

static void personagem_de_frente(int assento, Rectangle caixa) {
    RecursoId id = recurso_personagem(cena_personagem_do_assento(assento), VISTA_FRENTE, EXPRESSAO_NEUTRO);
    recurso_desenhar(id, caber(caixa, recurso_tamanho(id)), 0, WHITE);
}

/* O convite para clicar acende junto com o fim da trava: enquanto esta
   apagado, clicar nao adianta, e a tela mostra isso sem precisar de aviso. */
static float convite(float tempo, float trava) {
    float a = (trava > 0.0f) ? tempo / trava : 1.0f;
    return a > 1.0f ? 1.0f : a;
}

void telas_cobertura(int assento, const char *nome, float tempo, float trava) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);

    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, COR_COBERTURA);
    /* Vista de frente: na cobertura o jogador "olha" para quem vai pegar o
       computador, como o parceiro na mesa. */
    personagem_de_frente(assento, COBERTURA_PERSONAGEM);
    texto_centro(texto, "VEZ DE", 670.0f, 40.0f, COR_MOLDURA);
    texto_centro(titulo, nome, 714.0f, 110.0f, COR_TEXTO);
    texto_centro(texto, "clique ou ESPAÇO para ver suas cartas", 880.0f, 46.0f,
                 Fade(COR_TEXTO, convite(tempo, trava)));
}

void telas_resultado_rodada(ResultadoVisto resultado, float tempo) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    Rectangle r = TELAS_RESULTADO_RODADA;
    float cx = r.x + r.width * 0.5f;
    float a = tempo / TEMPO_SURGIR;
    const char *rotulo = "Empate";
    Color cor = HUD_COR_EMPATE;

    if (a > 1.0f) a = 1.0f;
    if (resultado == RODADA_NOS) { rotulo = "Nós"; cor = HUD_COR_NOS; }
    if (resultado == RODADA_ELES) { rotulo = "Eles"; cor = HUD_COR_ELES; }

    DrawRectangleRounded(r, 0.2f, 10, Fade(COR_PAINEL, a));
    DrawRectangleRoundedLinesEx(r, 0.2f, 10, 4.0f, Fade(cor, a));
    texto_em(texto, "Rodada:", cx, r.y + 12.0f, 36.0f, Fade(COR_TEXTO, a));
    texto_em(titulo, rotulo, cx, r.y + 52.0f, 78.0f, Fade(cor, a));
}

void telas_fim_mao(const char *duplaVencedora, const char *duplaCorreu, int pontos, float tempo, float trava) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    /* Termina em y 580, acima da cruz (que comeca em 606): a ultima rodada,
       com o realce de quem ganhou, continua aparecendo embaixo. */
    Rectangle faixa = { 0, 260, TELA_LARGURA, 320 };

    /* A mesa continua visivel por baixo (a ultima rodada e o placar), so
       mais escura: o recado e o que importa agora. */
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.45f));
    DrawRectangleRec(faixa, COR_PAINEL);
    DrawLineEx((Vector2){ 0, faixa.y }, (Vector2){ TELA_LARGURA, faixa.y }, 4.0f, COR_MOLDURA);
    DrawLineEx((Vector2){ 0, faixa.y + faixa.height }, (Vector2){ TELA_LARGURA, faixa.y + faixa.height },
               4.0f, COR_MOLDURA);

    texto_centro(texto, "FIM DA MÃO", faixa.y + 28.0f, 40.0f, COR_MOLDURA);
    if (duplaVencedora == NULL) {
        texto_centro(titulo, "Mão empatada: ninguém pontua", faixa.y + 92.0f, 84.0f, COR_TEXTO);
    } else if (duplaCorreu != NULL) {
        /* Dupla e plural: "Dona Marlene e Kiko correram". */
        texto_centro(titulo, TextFormat("%s correram", duplaCorreu), faixa.y + 82.0f, 76.0f, COR_TEXTO);
        texto_centro(texto, TextFormat("— %s levaram %d %s", duplaVencedora, pontos, pontos == 1 ? "ponto" : "pontos"),
                     faixa.y + 172.0f, 48.0f, COR_TEXTO);
    } else {
        texto_centro(titulo, duplaVencedora, faixa.y + 82.0f, 84.0f, COR_TEXTO);
        texto_centro(texto, TextFormat("levaram %d %s", pontos, pontos == 1 ? "ponto" : "pontos"),
                     faixa.y + 172.0f, 48.0f, COR_TEXTO);
    }
    texto_centro(texto, "clique ou ESPAÇO para continuar", faixa.y + 250.0f, 38.0f,
                 Fade(COR_MOLDURA, convite(tempo, trava)));
}

void telas_fim_partida(int assentoA, int assentoB, const char *duplaVencedora,
                       int pontosVencedora, int pontosPerdedora, float tempo, float trava) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);

    /* Opaca como a cobertura: a partida acabou, a mesa nao interessa mais. */
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, COR_COBERTURA);
    personagem_de_frente(assentoA, FIM_PERSONAGEM_A);
    personagem_de_frente(assentoB, FIM_PERSONAGEM_B);

    texto_centro(texto, "FIM DE PARTIDA", 580.0f, 44.0f, COR_MOLDURA);
    texto_centro(titulo, duplaVencedora, 632.0f, 100.0f, COR_TEXTO);
    texto_centro(texto, "venceram!", 744.0f, 52.0f, COR_TEXTO);
    texto_centro(titulo, TextFormat("%d x %d", pontosVencedora, pontosPerdedora), 812.0f, 72.0f,
                 COR_MOLDURA);
    texto_centro(texto, "clique para jogar de novo", 940.0f, 46.0f,
                 Fade(COR_TEXTO, convite(tempo, trava)));
}

/* ----------------------------------------------------------------- truco */

static const Color COR_TRUCO = { 236, 92, 44, 255 };   /* laranja do botao de pedir */

#define TEMPO_ENTRAR 0.25f   /* a palavra do anuncio entra crescendo */

void telas_anuncio_pedido(const char *nomePediu, const char *nomeValor, float tempo) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    const char *palavra = TextFormat("%s!", nomeValor);
    float a = tempo / TEMPO_ENTRAR, tamanho;
    Vector2 medida, pos;

    if (a > 1.0f) a = 1.0f;
    /* Entra grande e assenta (1,4x ate 1x): chama o olho de toda a mesa,
       como o grito de truco. */
    tamanho = 230.0f * (1.4f - 0.4f * a);
    medida = MeasureTextEx(titulo, palavra, tamanho, 1.0f);
    pos = (Vector2){ TELA_LARGURA * 0.5f - medida.x * 0.5f, 470.0f - medida.y * 0.5f };

    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.45f * a));
    texto_centro(texto, TextFormat("%s pediu", nomePediu), 250.0f, 56.0f, Fade(COR_TEXTO, a));
    /* Sombra em vez de contorno: a fonte de titulo ja e grossa. */
    DrawTextEx(titulo, palavra, (Vector2){ pos.x + 8.0f, pos.y + 10.0f }, tamanho, 1.0f, Fade(BLACK, 0.6f * a));
    DrawTextEx(titulo, palavra, pos, tamanho, 1.0f, Fade(COR_TRUCO, a));
}

void telas_cobertura_resposta(int assentoResponde, const char *nomeResponde, const char *nomePediu,
                              const char *nomeValor, float tempo, float trava) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);

    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, COR_COBERTURA);
    personagem_de_frente(assentoResponde, (Rectangle){ 760, 60, 400, 460 });
    texto_centro(titulo, TextFormat("%s pediu %s!", nomePediu, nomeValor), 540.0f, 64.0f, COR_TRUCO);
    texto_centro(texto, "VEZ DE", 650.0f, 40.0f, COR_MOLDURA);
    texto_centro(titulo, nomeResponde, 694.0f, 110.0f, COR_TEXTO);
    texto_centro(texto, "responder", 820.0f, 46.0f, COR_MOLDURA);
    texto_centro(texto, "clique ou ESPAÇO para ver suas cartas", 920.0f, 46.0f,
                 Fade(COR_TEXTO, convite(tempo, trava)));
}

void telas_pedido_pendente(const char *nomePediu, const char *nomeValor) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    Rectangle r = TELAS_RESULTADO_RODADA;
    float cx = r.x + r.width * 0.5f;

    DrawRectangleRounded(r, 0.2f, 10, COR_PAINEL);
    DrawRectangleRoundedLinesEx(r, 0.2f, 10, 4.0f, COR_TRUCO);
    texto_em(texto, TextFormat("%s pediu", nomePediu), cx, r.y + 14.0f, 32.0f, COR_TEXTO);
    texto_em(titulo, TextFormat("%s!", nomeValor), cx, r.y + 52.0f, 74.0f, COR_TRUCO);
}
