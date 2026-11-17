#include <stdio.h>
#include <string.h>

#include "recursos.h"

#define LADO_MAXIMO_TEXTURA 4096   /* limite combinado; acima disso algumas
                                      placas de video antigas recusam a textura */
#define TAMANHO_CAMINHO     512

/* Gera os 6 nomes de um personagem na mesma ordem do enum Vista/Expressao. */
#define NOMES_PERSONAGEM(N)                       \
    "personagens/p" #N "_frente_neutro.png",      \
    "personagens/p" #N "_frente_piscando.png",    \
    "personagens/p" #N "_frente_falando.png",     \
    "personagens/p" #N "_lado_neutro.png",        \
    "personagens/p" #N "_lado_piscando.png",      \
    "personagens/p" #N "_lado_falando.png"

/* Caminhos relativos a assets/, na mesma ordem do enum RecursoId. */
static const char *nomes[REC_TOTAL] = {
    "fundo/bar_panorama.png",
    "fundo/mesa.png",
    NOMES_PERSONAGEM(1),
    NOMES_PERSONAGEM(2),
    NOMES_PERSONAGEM(3),
    NOMES_PERSONAGEM(4),
    "paineis/lousa.png",
    "paineis/tv_var.png",
    "paineis/mural.png",
    "paineis/caderneta.png",
    "ui/logo.png",
    "ui/verso_carta.png",
};

static const char *nomesFontes[FONTE_TOTAL] = {
    "ui/fonte_titulo.ttf",
    "ui/fonte_texto.ttf",
    "ui/fonte_naipes.ttf",
};

/* Tamanho em que cada fonte e gerada. Grande de proposito: reduzir uma letra
   grande fica nitido, ampliar uma pequena fica borrado. */
static const int tamanhosFontes[FONTE_TOTAL] = { 128, 96, 128 };

static Texture2D texturas[REC_TOTAL];
static int carregada[REC_TOTAL];
static Font fontes[FONTE_TOTAL];
static int fonteCarregada[FONTE_TOTAL];

/* O executavel fica em output/ e os recursos em assets/, irmaos na raiz do
   repositorio. Montar o caminho a partir do executavel (e nao da pasta
   atual) faz o jogo achar os recursos mesmo aberto com duplo clique. */
static void montar_caminho(char *destino, const char *relativo) {
    snprintf(destino, TAMANHO_CAMINHO, "%s../assets/%s", GetApplicationDirectory(), relativo);
}

static void carregar_textura(RecursoId id) {
    char caminho[TAMANHO_CAMINHO];
    Image imagem;

    montar_caminho(caminho, nomes[id]);
    if (!FileExists(caminho)) {
        TraceLog(LOG_WARNING, "RECURSO: falta assets/%s (usando marcador provisorio)", nomes[id]);
        return;
    }
    imagem = LoadImage(caminho);
    if (imagem.data == NULL) {
        TraceLog(LOG_WARNING, "RECURSO: assets/%s nao abriu (arquivo corrompido?)", nomes[id]);
        return;
    }
    if (imagem.width > LADO_MAXIMO_TEXTURA || imagem.height > LADO_MAXIMO_TEXTURA) {
        /* Recusa em vez de reduzir sozinho: o aviso faz a arte ser corrigida
           na origem, e o jogo nao gasta tempo redimensionando na abertura. */
        TraceLog(LOG_WARNING, "RECURSO: assets/%s tem %dx%d px; o maximo e %d de lado",
                 nomes[id], imagem.width, imagem.height, LADO_MAXIMO_TEXTURA);
        UnloadImage(imagem);
        return;
    }
    texturas[id] = LoadTextureFromImage(imagem);
    UnloadImage(imagem);
    if (!IsTextureValid(texturas[id])) {
        TraceLog(LOG_WARNING, "RECURSO: assets/%s nao foi para a placa de video", nomes[id]);
        return;
    }
    SetTextureFilter(texturas[id], TEXTURE_FILTER_BILINEAR);
    carregada[id] = 1;
}

/* Monta a lista de caracteres que cada fonte precisa gerar. A raylib so
   desenha os caracteres pedidos aqui; o resto vira "?". */
static int montar_codepoints(FonteId id, int *codepoints) {
    int n = 0;
    for (int c = 32; c <= 126; c++) codepoints[n++] = c;        /* ASCII visivel */
    if (id == FONTE_NAIPES) {
        codepoints[n++] = 0x2660;   /* espadas */
        codepoints[n++] = 0x2663;   /* paus    */
        codepoints[n++] = 0x2665;   /* copas   */
        codepoints[n++] = 0x2666;   /* ouros   */
    } else {
        /* Latin-1 (a0 a ff): acentos do portugues, como em MAO com til. */
        for (int c = 0xA0; c <= 0xFF; c++) codepoints[n++] = c;
        /* Pontuacao tipografica das falas e mensagens. */
        codepoints[n++] = 0x2014;   /* travessao   */
        codepoints[n++] = 0x2026;   /* reticencias */
        codepoints[n++] = 0x201C;   /* aspas curvas de abertura   */
        codepoints[n++] = 0x201D;   /* aspas curvas de fechamento */
    }
    return n;
}

/* Maior lista possivel: ASCII (95) + Latin-1 (96) + pontuacao (4). */
#define MAX_CODEPOINTS (95 + 96 + 4)

static void carregar_fonte(FonteId id) {
    char caminho[TAMANHO_CAMINHO];
    int codepoints[MAX_CODEPOINTS];
    int quantidade;

    fontes[id] = GetFontDefault();   /* garante uma fonte valida em qualquer caso */
    montar_caminho(caminho, nomesFontes[id]);
    if (!FileExists(caminho)) {
        TraceLog(LOG_WARNING, "RECURSO: falta assets/%s (usando a fonte padrao)", nomesFontes[id]);
        return;
    }
    quantidade = montar_codepoints(id, codepoints);
    fontes[id] = LoadFontEx(caminho, tamanhosFontes[id], codepoints, quantidade);
    /* Se a leitura falhar a raylib devolve a fonte padrao sem dar erro:
       comparar a textura e o jeito de saber. */
    if (fontes[id].texture.id == GetFontDefault().texture.id) {
        TraceLog(LOG_WARNING, "RECURSO: assets/%s nao abriu (usando a fonte padrao)", nomesFontes[id]);
        return;
    }
    SetTextureFilter(fontes[id].texture, TEXTURE_FILTER_BILINEAR);
    fonteCarregada[id] = 1;
}

void recursos_carregar(void) {
    int faltando = 0;

    for (int i = 0; i < REC_TOTAL; i++) {
        carregar_textura((RecursoId)i);
        if (!carregada[i]) faltando++;
    }
    for (int i = 0; i < FONTE_TOTAL; i++) {
        carregar_fonte((FonteId)i);
        if (!fonteCarregada[i]) faltando++;
    }
    TraceLog(LOG_INFO, "RECURSO: %d de %d arquivos carregados",
             REC_TOTAL + FONTE_TOTAL - faltando, REC_TOTAL + FONTE_TOTAL);
}

void recursos_descarregar(void) {
    for (int i = 0; i < REC_TOTAL; i++) {
        if (carregada[i]) UnloadTexture(texturas[i]);
        carregada[i] = 0;
    }
    for (int i = 0; i < FONTE_TOTAL; i++) {
        /* A fonte padrao pertence a raylib: descarregar ela quebraria o texto. */
        if (fonteCarregada[i]) UnloadFont(fontes[i]);
        fonteCarregada[i] = 0;
    }
}

RecursoId recurso_personagem(int personagem, Vista vista, Expressao expressao) {
    if (personagem < 1 || personagem > 4) personagem = 1;   /* nunca sai da tabela */
    return (RecursoId)(REC_PERSONAGENS_INICIO + (personagem - 1) * 6 + vista * 3 + expressao);
}

int recurso_existe(RecursoId id) {
    return (int)id >= 0 && (int)id < REC_TOTAL && carregada[id];
}

const char *recurso_nome(RecursoId id) {
    if ((int)id < 0 || (int)id >= REC_TOTAL) return "?";
    return nomes[id];
}

Vector2 recurso_tamanho(RecursoId id) {
    Vector2 tamanho = { 0.0f, 0.0f };
    if (recurso_existe(id)) {
        tamanho.x = (float)texturas[id].width;
        tamanho.y = (float)texturas[id].height;
    }
    return tamanho;
}

/* Cor fixa por nome (hash djb2): o mesmo arquivo sempre tem a mesma cor, o
   que ajuda a reconhecer cada marcador de relance entre uma execucao e outra. */
static Color cor_do_nome(const char *nome) {
    unsigned long hash = 5381;
    while (*nome) hash = hash * 33 + (unsigned char)*nome++;
    return ColorFromHSV((float)(hash % 360), 0.45f, 0.70f);
}

static void desenhar_marcador(RecursoId id, Rectangle destino, Color tinta) {
    const char *nome = recurso_nome(id);
    Font fonte = recurso_fonte(FONTE_TEXTO);
    Color cor = cor_do_nome(nome);
    float tamanho = 36.0f;
    Vector2 medida;

    /* A tinta tambem vale para o marcador: quando a cena escurecer ou fizer
       fade, o provisorio se comporta como a imagem real. */
    cor = ColorTint(cor, tinta);
    cor.a = (unsigned char)(220 * tinta.a / 255);
    DrawRectangleRec(destino, cor);
    DrawRectangleLinesEx(destino, 3.0f, Fade(BLACK, 0.6f * tinta.a / 255.0f));

    /* Diminui o texto ate caber na largura (com folga para a borda). */
    medida = MeasureTextEx(fonte, nome, tamanho, 1.0f);
    while (medida.x > destino.width - 16.0f && tamanho > 10.0f) {
        tamanho -= 2.0f;
        medida = MeasureTextEx(fonte, nome, tamanho, 1.0f);
    }
    DrawTextEx(fonte, nome,
               (Vector2){ destino.x + (destino.width - medida.x) * 0.5f,
                          destino.y + (destino.height - medida.y) * 0.5f },
               tamanho, 1.0f, Fade(BLACK, tinta.a / 255.0f));
}

void recurso_desenhar(RecursoId id, Rectangle destino, int espelhado, Color tinta) {
    Rectangle origem;

    if (!recurso_existe(id)) {
        desenhar_marcador(id, destino, tinta);
        return;
    }
    /* Largura negativa na origem espelha a imagem sem precisar de outro arquivo. */
    origem = (Rectangle){ 0.0f, 0.0f, (float)texturas[id].width, (float)texturas[id].height };
    if (espelhado) origem.width = -origem.width;
    DrawTexturePro(texturas[id], origem, destino, (Vector2){ 0.0f, 0.0f }, 0.0f, tinta);
}

void recurso_desenhar_girado(RecursoId id, Vector2 centro, Vector2 tamanho,
                             float graus, Color tinta) {
    /* Na raylib a rotacao e em torno de "origin", medido a partir do canto do
       destino; com origin = metade do tamanho e destino no centro, gira em
       torno do centro. */
    Rectangle destino = { centro.x, centro.y, tamanho.x, tamanho.y };
    Vector2 meio = { tamanho.x * 0.5f, tamanho.y * 0.5f };

    if (!recurso_existe(id)) {
        Font fonte = recurso_fonte(FONTE_TEXTO);
        Color cor = ColorTint(cor_do_nome(recurso_nome(id)), tinta);
        float letra = 14.0f;
        Vector2 medida = MeasureTextEx(fonte, recurso_nome(id), letra, 1.0f);
        cor.a = (unsigned char)(220 * tinta.a / 255);
        DrawRectanglePro(destino, meio, graus, cor);
        /* Texto pequeno e girado junto: so para identificar, o marcador e
           do tamanho de uma carta. */
        DrawTextPro(fonte, recurso_nome(id), centro,
                    (Vector2){ medida.x * 0.5f, medida.y * 0.5f }, graus + 90.0f,
                    letra, 1.0f, Fade(BLACK, tinta.a / 255.0f));
        return;
    }
    DrawTexturePro(texturas[id],
                   (Rectangle){ 0.0f, 0.0f, (float)texturas[id].width, (float)texturas[id].height },
                   destino, meio, graus, tinta);
}

Font recurso_fonte(FonteId id) {
    if ((int)id < 0 || (int)id >= FONTE_TOTAL) return GetFontDefault();
    /* Antes do recursos_carregar o vetor ainda esta zerado: cai na padrao. */
    if (!fonteCarregada[id]) return GetFontDefault();
    return fontes[id];
}

int recurso_fonte_existe(FonteId id) {
    return (int)id >= 0 && (int)id < FONTE_TOTAL && fonteCarregada[id];
}
