#include "cena.h"
#include "recursos.h"
#include "tela.h"

int cena_assento_na_posicao(int assentoVisto, PosicaoNaTela posicao) {
    /* POSICAO_DIREITA = 0, FRENTE = 1, ESQUERDA = 2: somar posicao + 1 da
       exatamente +1, +2 e +3 no sentido do jogo. */
    return (assentoVisto + (int)posicao + 1) % 4;
}

int cena_personagem_do_assento(int assento) {
    return assento + 1;
}

/* Maior retangulo com a proporcao da imagem que cabe na caixa, apoiado na
   base e centralizado: a arte nunca deforma, e os pes ficam sempre na mesma
   linha, qualquer que seja o tamanho do arquivo. Sem imagem, usa a caixa toda. */
static Rectangle caber_na_caixa(Rectangle caixa, Vector2 tamanhoImagem) {
    Rectangle r = caixa;
    float escala, ex, ey;

    if (tamanhoImagem.x <= 0.0f || tamanhoImagem.y <= 0.0f) return caixa;
    ex = caixa.width / tamanhoImagem.x;
    ey = caixa.height / tamanhoImagem.y;
    escala = (ex < ey) ? ex : ey;
    r.width = tamanhoImagem.x * escala;
    r.height = tamanhoImagem.y * escala;
    r.x = caixa.x + (caixa.width - r.width) * 0.5f;
    r.y = caixa.y + caixa.height - r.height;
    return r;
}

/* Camada 1: o panorama e ajustado pela altura (1080) mantendo a proporcao;
   a largura que sobra fica escondida dos lados e sera a margem da paralaxe.
   Se a imagem vier mais estreita que 16:9, ajusta pela largura para nunca
   aparecer buraco nas laterais. */
static void desenhar_panorama(void) {
    Vector2 tamanho = recurso_tamanho(REC_FUNDO_PANORAMA);
    float proporcao, largura, altura;

    proporcao = (tamanho.y > 0.0f) ? tamanho.x / tamanho.y : CENA_PANORAMA_PROPORCAO_PROVISORIA;
    altura = TELA_ALTURA;
    largura = altura * proporcao;
    if (largura < TELA_LARGURA) {
        largura = TELA_LARGURA;
        altura = largura / proporcao;
    }
    recurso_desenhar(REC_FUNDO_PANORAMA,
                     (Rectangle){ (TELA_LARGURA - largura) * 0.5f, (TELA_ALTURA - altura) * 0.5f,
                                  largura, altura },
                     0, WHITE);
}

/* Camada 2: na etapa 1 so marca as areas; a lousa, a TV, o mural e a
   caderneta de verdade ja estarao pintados no panorama. */
static void desenhar_paineis(void) {
    recurso_desenhar(REC_PAINEL_LOUSA, CENA_PAINEL_LOUSA, 0, WHITE);
    recurso_desenhar(REC_PAINEL_MURAL, CENA_PAINEL_MURAL, 0, WHITE);
    recurso_desenhar(REC_PAINEL_TV_VAR, CENA_PAINEL_TV_VAR, 0, WHITE);
    recurso_desenhar(REC_PAINEL_CADERNETA, CENA_PAINEL_CADERNETA, 0, WHITE);
}

/* PROVISORIO: enquanto a arte nao chega, escreve sobre o marcador quem e o
   personagem e de que assento, para conferir a rotacao da mesa. */
static void rotular_personagem(Rectangle caixa, int personagem, int assento) {
    Font fonte = recurso_fonte(FONTE_TEXTO);
    const char *texto = TextFormat("p%d — assento %d", personagem, assento);
    float tamanho = 44.0f;
    Vector2 medida = MeasureTextEx(fonte, texto, tamanho, 1.0f);
    Vector2 posicao = { caixa.x + (caixa.width - medida.x) * 0.5f, caixa.y + 40.0f };

    DrawRectangleRounded((Rectangle){ posicao.x - 16, posicao.y - 8, medida.x + 32, medida.y + 16 },
                         0.4f, 8, Fade(BLACK, 0.65f));
    DrawTextEx(fonte, texto, posicao, tamanho, 1.0f, RAYWHITE);
}

static void desenhar_personagem(int assentoVisto, PosicaoNaTela posicao) {
    int assento = cena_assento_na_posicao(assentoVisto, posicao);
    int personagem = cena_personagem_do_assento(assento);
    Vista vista = (posicao == POSICAO_FRENTE) ? VISTA_FRENTE : VISTA_LADO;
    RecursoId id = recurso_personagem(personagem, vista, EXPRESSAO_NEUTRO);
    Rectangle caixa;
    int espelhado = 0;

    if (posicao == POSICAO_ESQUERDA) {
        caixa = CENA_PERSONAGEM_ESQUERDA;
        espelhado = !CENA_ARTE_LADO_OLHA_PARA_DIREITA;
    } else if (posicao == POSICAO_DIREITA) {
        caixa = CENA_PERSONAGEM_DIREITA;
        espelhado = CENA_ARTE_LADO_OLHA_PARA_DIREITA;
    } else {
        caixa = CENA_PERSONAGEM_FRENTE;
    }

    recurso_desenhar(id, caber_na_caixa(caixa, recurso_tamanho(id)), espelhado, WHITE);
    if (!recurso_existe(id)) rotular_personagem(caixa, personagem, assento);
}

/* Camada 3: o parceiro (frente) primeiro, porque esta mais longe; os
   laterais por cima dele, caso as imagens se encostem. */
static void desenhar_personagens(int assentoVisto) {
    desenhar_personagem(assentoVisto, POSICAO_FRENTE);
    desenhar_personagem(assentoVisto, POSICAO_ESQUERDA);
    desenhar_personagem(assentoVisto, POSICAO_DIREITA);
}

/* Camada 4: a mesa por cima dos personagens esconde a parte de baixo deles. */
static void desenhar_mesa(void) {
    recurso_desenhar(REC_FUNDO_MESA, CENA_MESA, 0, WHITE);
}

void cena_desenhar(int assentoVisto) {
    desenhar_panorama();
    desenhar_paineis();
    desenhar_personagens(assentoVisto);
    desenhar_mesa();
}
