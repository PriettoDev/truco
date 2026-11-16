#include "tela.h"

/*
 * Por que a RenderTexture nao tem tamanho fixo de 1920x1080:
 * ampliar ou reduzir uma imagem pronta sempre borra um pouco. Em vez disso a
 * textura tem o tamanho, em pixels reais, da area onde a cena aparece, e uma
 * Camera2D com zoom converte as coordenadas logicas (1920x1080) para esses
 * pixels. Assim cada pixel da textura cai em um pixel da tela: nitido em
 * Retina, em monitor comum e em qualquer tamanho de janela.
 *
 * Duas unidades diferentes aparecem aqui (em Retina elas diferem):
 *  - pontos de tela: GetScreenWidth e GetMousePosition (tamanho da janela);
 *  - pixels reais:   GetRenderWidth (tamanho do framebuffer, 2x em Retina).
 * O letterbox e o mouse sao calculados em pontos de tela; so o tamanho da
 * textura usa pixels reais.
 */

static RenderTexture2D alvo;
static int alvoCarregado = 0;
static Rectangle destino;     /* area da cena na janela, em pontos de tela */
static float escala = 1.0f;   /* pontos de tela por unidade logica */

/* Maior retangulo 16:9 que cabe na janela, centralizado: o que sobra vira
   barra preta, em cima/embaixo ou nas laterais. */
static void calcular_letterbox(void) {
    float largura = (float)GetScreenWidth();
    float altura  = (float)GetScreenHeight();
    float ex, ey;

    /* Janela minimizada pode informar tamanho 0: evita divisao por zero. */
    if (largura < 1.0f) largura = 1.0f;
    if (altura  < 1.0f) altura  = 1.0f;

    ex = largura / TELA_LARGURA;
    ey = altura / TELA_ALTURA;
    escala = (ex < ey) ? ex : ey;

    destino.width  = TELA_LARGURA * escala;
    destino.height = TELA_ALTURA * escala;
    destino.x = (largura - destino.width) * 0.5f;
    destino.y = (altura - destino.height) * 0.5f;
}

/* Recria a textura so quando o tamanho em pixels muda (redimensionar,
   trocar de monitor, tela cheia); recriar todo quadro seria desperdicio. */
static void ajustar_alvo(void) {
    float pixelsPorPonto = (float)GetRenderWidth() / (float)GetScreenWidth();
    int largura, altura;

    if (GetScreenWidth() <= 0) pixelsPorPonto = 1.0f;
    largura = (int)(destino.width * pixelsPorPonto + 0.5f);
    altura  = (int)(destino.height * pixelsPorPonto + 0.5f);
    if (largura < 1) largura = 1;
    if (altura  < 1) altura  = 1;

    if (alvoCarregado && alvo.texture.width == largura && alvo.texture.height == altura)
        return;
    if (alvoCarregado) UnloadRenderTexture(alvo);
    alvo = LoadRenderTexture(largura, altura);
    /* Bilinear suaviza o arredondamento de meio pixel entre textura e tela. */
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_BILINEAR);
    alvoCarregado = 1;
}

void tela_iniciar(void) {
    calcular_letterbox();
    ajustar_alvo();
}

void tela_encerrar(void) {
    if (alvoCarregado) UnloadRenderTexture(alvo);
    alvoCarregado = 0;
}

void tela_comecar_desenho(void) {
    Camera2D camera = { 0 };

    /* Recalcula todo quadro: e barato e nao depende de IsWindowResized, que
       nem sempre avisa quando se entra ou sai de tela cheia. */
    calcular_letterbox();
    ajustar_alvo();

    camera.zoom = (float)alvo.texture.width / TELA_LARGURA;
    BeginTextureMode(alvo);
    ClearBackground(BLACK);
    BeginMode2D(camera);
}

void tela_terminar_desenho(void) {
    /* A RenderTexture fica de cabeca para baixo no OpenGL: altura negativa
       na origem desvira a imagem. */
    Rectangle origem = { 0.0f, 0.0f, (float)alvo.texture.width, -(float)alvo.texture.height };

    EndMode2D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);   /* as barras pretas do letterbox */
    DrawTexturePro(alvo.texture, origem, destino, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    EndDrawing();
}

Vector2 tela_janela_para_logico(Vector2 ponto) {
    Vector2 logico;
    calcular_letterbox();   /* garante o valor certo mesmo antes do 1o desenho */
    logico.x = (ponto.x - destino.x) / escala;
    logico.y = (ponto.y - destino.y) / escala;
    return logico;
}

Vector2 tela_mouse(void) {
    return tela_janela_para_logico(GetMousePosition());
}

/* Testado em Retina (raylib 6.2): ToggleFullscreen e ToggleBorderlessWindowed
   trocam a janela para 1920x1200 em escala 1 (perde o HiDPI e o macOS amplia
   a imagem, borrando), e uma janela sem borda feita a mao fica 33 pontos
   abaixo da barra de menus, cortando o rodape. Por isso no macOS o F11
   maximiza/restaura (mantem o Retina); a tela cheia de verdade no Mac e o
   botao verde ou Ctrl+Cmd+F, que chegam aqui como redimensionamento comum.
   No Windows (apresentacao) a tela cheia sem borda nao troca a resolucao do
   monitor, o que evita piscar o projetor. */
void tela_alternar_tela_cheia(void) {
#ifdef __APPLE__
    if (IsWindowMaximized()) RestoreWindow();
    else MaximizeWindow();
#else
    ToggleBorderlessWindowed();
#endif
}
