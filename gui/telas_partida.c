#include "raylib.h"

#include "telas_partida.h"
#include "tela.h"
#include "recursos.h"
#include "cena.h"

static const Color COR_COBERTURA = {  18,  14,  12, 246 };   /* quase opaca: nada da mesa vaza */
static const Color COR_MOLDURA   = { 214, 170,  80, 255 };
static const Color COR_TEXTO     = { 245, 236, 214, 255 };

/* Caixa do personagem na cobertura: no meio, acima do nome. */
#define COBERTURA_PERSONAGEM ((Rectangle){ 760, 120, 400, 520 })

static void texto_centro(Font fonte, const char *texto, float y, float tamanho, Color cor) {
    Vector2 medida = MeasureTextEx(fonte, texto, tamanho, 1.0f);
    DrawTextEx(fonte, texto, (Vector2){ TELA_LARGURA * 0.5f - medida.x * 0.5f, y }, tamanho, 1.0f, cor);
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

void telas_cobertura(int assento, const char *nome, float tempo, float trava) {
    Font titulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    /* Vista de frente: na cobertura o jogador "olha" para quem vai pegar o
       computador, como o parceiro na mesa. */
    RecursoId id = recurso_personagem(cena_personagem_do_assento(assento), VISTA_FRENTE, EXPRESSAO_NEUTRO);
    float convite = (trava > 0.0f) ? tempo / trava : 1.0f;

    if (convite > 1.0f) convite = 1.0f;
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, COR_COBERTURA);

    recurso_desenhar(id, caber(COBERTURA_PERSONAGEM, recurso_tamanho(id)), 0, WHITE);
    texto_centro(texto, "VEZ DE", 670.0f, 40.0f, COR_MOLDURA);
    texto_centro(titulo, nome, 714.0f, 110.0f, COR_TEXTO);
    /* Aparece junto com o fim da trava: enquanto esta apagado, clicar nao
       adianta, e a tela mostra isso sem precisar de mensagem. */
    texto_centro(texto, "clique ou ESPAÇO para ver suas cartas", 880.0f, 46.0f, Fade(COR_TEXTO, convite));
}

void telas_aviso_provisorio(const char *titulo, const char *convite) {
    Font fonteTitulo = recurso_fonte(FONTE_TITULO);
    Font texto = recurso_fonte(FONTE_TEXTO);
    Rectangle faixa = { 0, 400, TELA_LARGURA, 220 };

    DrawRectangleRec(faixa, Fade(COR_COBERTURA, 0.85f));
    texto_centro(fonteTitulo, titulo, faixa.y + 30.0f, 90.0f, COR_TEXTO);
    texto_centro(texto, convite, faixa.y + 140.0f, 42.0f, COR_MOLDURA);
}
