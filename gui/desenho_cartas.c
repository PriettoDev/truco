#include <math.h>   /* expf, para a animacao independente do FPS */

#include "desenho_cartas.h"
#include "recursos.h"
#include "rlgl.h"    /* matriz de rotacao para a face girada; depois do raylib.h */

#define MAX_CARTAS_MAO 3

/* Cores da face. Vermelho e preto vem do naipe (enum), nao do texto, para
   nao depender de comparar simbolos UTF-8. */
static const Color COR_PAPEL    = { 250, 246, 236, 255 };
static const Color COR_BORDA    = {  70,  64,  58, 255 };
static const Color COR_VERMELHO = { 196,  30,  45, 255 };
static const Color COR_PRETO    = {  28,  28,  32, 255 };

/* Quanto cada carta da mao esta levantada agora (0 ate DESENHO_MAO_ELEVACAO). */
static float elevacao[MAX_CARTAS_MAO];

static Color cor_do_naipe(Naipe naipe) {
    return (naipe == OUROS || naipe == COPAS) ? COR_VERMELHO : COR_PRETO;
}

/* Valor e naipe empilhados e centralizados numa coluna, como no canto de um
   baralho de verdade. rotacao 180 desenha o canto de baixo, de ponta-cabeca. */
static void desenhar_canto(Carta carta, Vector2 ponto, float largura, float rotacao) {
    Font fonteValor = recurso_fonte(FONTE_TITULO);
    Font fonteNaipe = recurso_fonte(FONTE_NAIPES);
    const char *valor = valor_texto(carta.valor);
    const char *naipe = naipe_texto(carta.naipe);
    float tamValor = largura * 0.26f;
    float tamNaipe = largura * 0.19f;
    Vector2 medValor = MeasureTextEx(fonteValor, valor, tamValor, 0.0f);
    Vector2 medNaipe = MeasureTextEx(fonteNaipe, naipe, tamNaipe, 0.0f);
    float coluna = largura * 0.14f;   /* centro da coluna, a partir da borda */
    Color cor = cor_do_naipe(carta.naipe);

    /* origin e medido a partir do ponto: com rotacao 180 o mesmo desenho
       "cresce" para cima e para a esquerda, espelhando o canto de cima. */
    DrawTextPro(fonteValor, valor, ponto,
                (Vector2){ -(coluna - medValor.x * 0.5f), 0.0f }, rotacao, tamValor, 0.0f, cor);
    DrawTextPro(fonteNaipe, naipe, ponto,
                (Vector2){ -(coluna - medNaipe.x * 0.5f), -medValor.y * 0.92f },
                rotacao, tamNaipe, 0.0f, cor);
}

void desenho_carta_face(Carta carta, Rectangle area) {
    Font fonteNaipe = recurso_fonte(FONTE_NAIPES);
    const char *naipe = naipe_texto(carta.naipe);
    float largura = area.width;
    float margem = largura * 0.05f;
    float tamCentro = largura * 0.62f;
    Vector2 medCentro = MeasureTextEx(fonteNaipe, naipe, tamCentro, 0.0f);

    /* Sombra deslocada: separa a carta da mesa sem precisar de imagem. */
    DrawRectangleRounded((Rectangle){ area.x + largura * 0.03f, area.y + largura * 0.04f,
                                      area.width, area.height },
                         0.12f, 12, Fade(BLACK, 0.35f));
    DrawRectangleRounded(area, 0.12f, 12, COR_PAPEL);
    DrawRectangleRoundedLinesEx(area, 0.12f, 12, largura * 0.012f, COR_BORDA);

    desenhar_canto(carta, (Vector2){ area.x + margem * 0.4f, area.y + margem }, largura, 0.0f);
    desenhar_canto(carta, (Vector2){ area.x + area.width - margem * 0.4f,
                                     area.y + area.height - margem },
                   largura, 180.0f);

    DrawTextEx(fonteNaipe, naipe,
               (Vector2){ area.x + (area.width - medCentro.x) * 0.5f,
                          area.y + (area.height - medCentro.y) * 0.5f },
               tamCentro, 0.0f, cor_do_naipe(carta.naipe));
}

void desenho_carta_face_girada(Carta carta, Vector2 centro, float largura, float graus) {
    float altura = largura * DESENHO_CARTA_PROPORCAO;

    /* Gira o sistema de coordenadas em vez de reescrever a face: a carta e
       desenhada centrada na origem e a matriz leva para o lugar certo. Os
       cantos ja sao simetricos (o de baixo e de ponta-cabeca), entao a carta
       se le de qualquer lado da mesa, como um baralho de verdade. */
    rlPushMatrix();
    rlTranslatef(centro.x, centro.y, 0.0f);
    rlRotatef(graus, 0.0f, 0.0f, 1.0f);
    desenho_carta_face(carta, (Rectangle){ -largura * 0.5f, -altura * 0.5f, largura, altura });
    rlPopMatrix();
}

/* Centro e rotacao de cada lugar da cruz. Mesma rotacao dos versos de cada
   lado: a carta fica "de frente" para quem a jogou. */
static void lugar_na_cruz(LadoDaMesa lado, Vector2 *centro, float *graus) {
    Vector2 c = DESENHO_JOGADA_CENTRO;
    float g = 0.0f;

    switch (lado) {
    case LADO_EU:       c.y += DESENHO_JOGADA_AFASTAMENTO_V; break;
    case LADO_FRENTE:   c.y -= DESENHO_JOGADA_AFASTAMENTO_V; g = DESENHO_VERSOS_FRENTE_GRAUS; break;
    case LADO_ESQUERDA: c.x -= DESENHO_JOGADA_AFASTAMENTO_H; g = DESENHO_VERSOS_ESQUERDA_GRAUS; break;
    case LADO_DIREITA:  c.x += DESENHO_JOGADA_AFASTAMENTO_H; g = DESENHO_VERSOS_DIREITA_GRAUS; break;
    }
    *centro = c;
    *graus = g;
}

void desenho_carta_jogada(Carta carta, LadoDaMesa lado) {
    Vector2 c;
    float graus;
    lugar_na_cruz(lado, &c, &graus);
    desenho_carta_face_girada(carta, c, DESENHO_JOGADA_LARGURA, graus);
}

Rectangle desenho_area_cartas_jogadas(void) {
    /* A carta deitada (lados) ocupa a altura na horizontal: por isso a meia
       altura entra nas duas direcoes. */
    float meiaAltura = DESENHO_JOGADA_LARGURA * DESENHO_CARTA_PROPORCAO * 0.5f;
    Vector2 c = DESENHO_JOGADA_CENTRO;
    float dx = DESENHO_JOGADA_AFASTAMENTO_H + meiaAltura;
    float dy = DESENHO_JOGADA_AFASTAMENTO_V + meiaAltura;
    return (Rectangle){ c.x - dx, c.y - dy, 2.0f * dx, 2.0f * dy };
}

/* Carta em voo: onde esta agora e para onde vai. */
static struct {
    int ativo;
    Carta carta;
    Vector2 centro, centroAlvo;
    float largura, larguraAlvo;
    float graus, grausAlvo;
} voo;

static Rectangle area_na_mao(int i, int quantidade);

void desenho_voo_iniciar(Carta carta, int posicaoNaMao, int quantidadeNaMao, LadoDaMesa destino) {
    Rectangle origem = area_na_mao(posicaoNaMao, quantidadeNaMao);

    /* Sai de onde a carta estava desenhada (com a elevacao do mouse), para
       nao dar salto no primeiro quadro. */
    voo.ativo = 1;
    voo.carta = carta;
    voo.centro = (Vector2){ origem.x + origem.width * 0.5f, origem.y + origem.height * 0.5f };
    voo.largura = origem.width;
    voo.graus = 0.0f;
    lugar_na_cruz(destino, &voo.centroAlvo, &voo.grausAlvo);
    voo.larguraAlvo = DESENHO_JOGADA_LARGURA;
}

void desenho_voo_atualizar(float dt) {
    float passo = 1.0f - expf(-DESENHO_VOO_SUAVIDADE * dt);
    if (!voo.ativo) return;
    voo.centro.x += (voo.centroAlvo.x - voo.centro.x) * passo;
    voo.centro.y += (voo.centroAlvo.y - voo.centro.y) * passo;
    voo.largura += (voo.larguraAlvo - voo.largura) * passo;
    voo.graus += (voo.grausAlvo - voo.graus) * passo;
}

void desenho_voo(void) {
    if (voo.ativo) desenho_carta_face_girada(voo.carta, voo.centro, voo.largura, voo.graus);
}

static float centro_x_na_mao(int i, int quantidade) {
    /* Centraliza qualquer quantidade (3, 2 ou 1 carta) no meio da tela. */
    return 960.0f + ((float)i - (quantidade - 1) * 0.5f) * DESENHO_MAO_ESPACO;
}

static Rectangle area_na_mao(int i, int quantidade) {
    float altura = DESENHO_MAO_LARGURA_CARTA * DESENHO_CARTA_PROPORCAO;
    return (Rectangle){ centro_x_na_mao(i, quantidade) - DESENHO_MAO_LARGURA_CARTA * 0.5f,
                        DESENHO_MAO_TOPO - elevacao[i],
                        DESENHO_MAO_LARGURA_CARTA, altura };
}

void desenho_mao_atualizar(int quantidade, Vector2 mouse, float dt) {
    /* Aproximacao exponencial: anda uma fracao do caminho a cada quadro,
       com a fracao calculada pelo tempo real. Fica suave e igual a 30 ou
       120 FPS, sem guardar inicio nem duracao de animacao. */
    float passo = 1.0f - expf(-DESENHO_MAO_SUAVIDADE * dt);

    if (quantidade > MAX_CARTAS_MAO) quantidade = MAX_CARTAS_MAO;
    for (int i = 0; i < quantidade; i++) {
        Rectangle area = area_na_mao(i, quantidade);
        float alvo;

        /* A area de toque vai da posicao levantada ate a base: sem isso, com
           o mouse na borda de baixo, a carta subiria, sairia de baixo do
           mouse, desceria e ficaria tremendo. */
        area.height += elevacao[i];
        alvo = CheckCollisionPointRec(mouse, area) ? DESENHO_MAO_ELEVACAO : 0.0f;
        elevacao[i] += (alvo - elevacao[i]) * passo;
    }
    for (int i = quantidade; i < MAX_CARTAS_MAO; i++) elevacao[i] = 0.0f;
}

void desenho_mao(const Carta cartas[], int quantidade) {
    if (quantidade > MAX_CARTAS_MAO) quantidade = MAX_CARTAS_MAO;
    for (int i = 0; i < quantidade; i++) {
        desenho_carta_face(cartas[i], area_na_mao(i, quantidade));
    }
}

void desenho_mao_versos(int quantidade) {
    if (quantidade > MAX_CARTAS_MAO) quantidade = MAX_CARTAS_MAO;
    for (int i = 0; i < quantidade; i++) {
        recurso_desenhar(REC_UI_VERSO_CARTA, area_na_mao(i, quantidade), 0, WHITE);
    }
}

int desenho_mao_carta_sob(int quantidade, Vector2 mouse) {
    if (quantidade > MAX_CARTAS_MAO) quantidade = MAX_CARTAS_MAO;
    for (int i = 0; i < quantidade; i++) {
        Rectangle area = area_na_mao(i, quantidade);
        area.height += elevacao[i];   /* igual ao destaque: sem buraco embaixo */
        if (CheckCollisionPointRec(mouse, area)) return i;
    }
    return -1;
}

void desenho_vira(Carta vira) {
    Font fonte = recurso_fonte(FONTE_TEXTO);
    float altura = DESENHO_VIRA_LARGURA * DESENHO_CARTA_PROPORCAO;
    Rectangle area = { DESENHO_VIRA_CENTRO.x - DESENHO_VIRA_LARGURA * 0.5f,
                       DESENHO_VIRA_CENTRO.y - altura * 0.5f,
                       DESENHO_VIRA_LARGURA, altura };
    Vector2 medida = MeasureTextEx(fonte, "VIRA", 30.0f, 1.0f);

    desenho_carta_face(vira, area);
    DrawTextEx(fonte, "VIRA",
               (Vector2){ DESENHO_VIRA_CENTRO.x - medida.x * 0.5f, area.y + altura + 8.0f },
               30.0f, 1.0f, RAYWHITE);
}

void desenho_monte(void) {
    Vector2 tamanho = { DESENHO_VIRA_LARGURA, DESENHO_VIRA_LARGURA * DESENHO_CARTA_PROPORCAO };

    /* Cada camada um pouco para cima e para a esquerda: parece uma pilha
       vista de cima, sem precisar de imagem de monte. */
    for (int i = 0; i < DESENHO_MONTE_CAMADAS; i++) {
        float d = (float)(DESENHO_MONTE_CAMADAS - 1 - i) * 3.0f;
        recurso_desenhar_girado(REC_UI_VERSO_CARTA,
                                (Vector2){ DESENHO_MONTE_CENTRO.x + d, DESENHO_MONTE_CENTRO.y + d },
                                tamanho, 0.0f, WHITE);
    }
}

Rectangle desenho_area_mao(int quantidade) {
    float altura = DESENHO_MAO_LARGURA_CARTA * DESENHO_CARTA_PROPORCAO;
    float esquerda, direita;

    if (quantidade < 1) return (Rectangle){ 0, 0, 0, 0 };
    if (quantidade > MAX_CARTAS_MAO) quantidade = MAX_CARTAS_MAO;
    esquerda = centro_x_na_mao(0, quantidade) - DESENHO_MAO_LARGURA_CARTA * 0.5f;
    direita = centro_x_na_mao(quantidade - 1, quantidade) + DESENHO_MAO_LARGURA_CARTA * 0.5f;
    /* Comeca na altura da carta levantada ao maximo: a area protegida vale
       em qualquer momento da animacao. */
    return (Rectangle){ esquerda, DESENHO_MAO_TOPO - DESENHO_MAO_ELEVACAO,
                        direita - esquerda, DESENHO_MAO_ELEVACAO + altura };
}

Rectangle desenho_area_vira_e_monte(void) {
    float altura = DESENHO_VIRA_LARGURA * DESENHO_CARTA_PROPORCAO;
    float esquerda = DESENHO_VIRA_CENTRO.x - DESENHO_VIRA_LARGURA * 0.5f;
    /* O monte se espalha ate (camadas-1)*3 px para a direita e para baixo. */
    float extra = (DESENHO_MONTE_CAMADAS - 1) * 3.0f;
    float direita = DESENHO_MONTE_CENTRO.x + DESENHO_VIRA_LARGURA * 0.5f + extra;

    /* Inclui o rotulo "VIRA" embaixo da carta (uns 40 px). */
    return (Rectangle){ esquerda, DESENHO_VIRA_CENTRO.y - altura * 0.5f,
                        direita - esquerda, altura + extra + 40.0f };
}

Rectangle desenho_area_versos_frente(void) {
    /* Carta de 80x120 inclinada ate 9 graus: meia altura 60*cos9 + 40*sin9
       (66) e meia largura do leque 0,55*80 + 66 (110). */
    Vector2 c = DESENHO_VERSOS_FRENTE_CENTRO;
    return (Rectangle){ c.x - 110.0f, c.y - 66.0f, 220.0f, 132.0f };
}

/* Leque de versos: cada carta um pouco deslocada e inclinada em relacao ao
   centro, e o leque inteiro girado para o lado de quem segura. */
static void desenhar_leque(int quantidade, Vector2 centro, float grausLeque) {
    Vector2 tamanho = { DESENHO_VERSO_LARGURA, DESENHO_VERSO_LARGURA * DESENHO_CARTA_PROPORCAO };
    float rad = grausLeque * DEG2RAD;

    for (int i = 0; i < quantidade; i++) {
        float posicao = (float)i - (quantidade - 1) * 0.5f;   /* -1, 0, +1 */
        float desloc = posicao * DESENHO_VERSO_LARGURA * 0.55f;
        /* Desloca ao longo do eixo do leque ja girado. */
        Vector2 c = { centro.x + desloc * cosf(rad), centro.y + desloc * sinf(rad) };
        recurso_desenhar_girado(REC_UI_VERSO_CARTA, c, tamanho, grausLeque + posicao * 9.0f, WHITE);
    }
}

void desenho_versos_dos_outros(int qtdDireita, int qtdFrente, int qtdEsquerda) {
    desenhar_leque(qtdFrente, DESENHO_VERSOS_FRENTE_CENTRO, DESENHO_VERSOS_FRENTE_GRAUS);
    desenhar_leque(qtdEsquerda, DESENHO_VERSOS_ESQUERDA_CENTRO, DESENHO_VERSOS_ESQUERDA_GRAUS);
    desenhar_leque(qtdDireita, DESENHO_VERSOS_DIREITA_CENTRO, DESENHO_VERSOS_DIREITA_GRAUS);
}
