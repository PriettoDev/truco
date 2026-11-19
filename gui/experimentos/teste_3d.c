/*
 * teste_3d.c - EXPERIMENTO: personagens 3D (modelos estaticos do Meshy) na
 * nossa cena 2D. Programa separado do jogo: make -C gui teste3d.
 *
 * Como encaixa na cena:
 *  - Os modelos sao desenhados numa RenderTexture propria, com fundo
 *    transparente, e a textura entra na camada dos personagens: depois do
 *    panorama e dos paineis, antes da mesa. Assim a mesa continua por cima
 *    e esconde a cintura, como com os personagens 2D.
 *  - A camera 3D e casada com a tela logica 1920x1080: a uma profundidade d,
 *    a altura visivel e 2*d*tan(fov/2), que corresponde a 1080 px. Com isso
 *    cada modelo e posto exatamente na caixa CENA_PERSONAGEM_* (centro em X,
 *    base e altura em pixels), e as constantes de layout continuam valendo.
 *  - cena.c e incluido aqui para usar as camadas (static) sem mexer nele.
 *
 * Teclas: C liga/desliga o descarte de faces de tras; ESC sai.
 * "--medir PASTA": mede (sem vsync), captura e sai. Usado no relatorio.
 */
#include <stdio.h>
#include <stdlib.h>   /* abs */
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "tela.h"
#include "recursos.h"
#include "desenho_cartas.h"
#include "hud.h"
#include "rodada.h"   /* INDEFINIDO */
#include "../cena.c"   /* camadas static: panorama, paineis, mesa */

/* RenderTexture 3D com o dobro da resolucao logica: a RenderTexture nao tem
   MSAA, entao a borda dos modelos e suavizada desenhando maior e reduzindo
   com filtro bilinear (superamostragem). */
#define SUPERAMOSTRAGEM 2

#define FOV_GRAUS 30.0f        /* lente estreita: pouca distorcao de perspectiva */
#define CAMERA_ALTURA 1.2f     /* olho de quem esta sentado a mesa */

/* Profundidade do parceiro (do outro lado da mesa). A dos outros sai da
   conta: mesma escala de mundo (vezes a escala relativa) e altura na tela
   igual a da caixa deles, entao ficam mais perto da camera. */
#define PROF_FRENTE  4.2f

#define ALTURA_MODELO 1.9f     /* preparar_modelo.py: Y de 0 a 1,9 */

/* Respiracao: 1% de escala em Y, a partir dos pes (origem do modelo). */
#define RESPIRACAO_AMPLITUDE 0.01f
#define RESPIRACAO_PERIODO   3.6f     /* segundos por ciclo */

typedef struct {
    Model modelo;
    Rectangle caixa;          /* caixa da cena onde ele deve caber */
    float profundidade;
    float escalaRelativa;     /* p1 1,00; p2 1,03 */
    float giroY;              /* graus: o da direita vira para o centro da mesa */
    float fase;               /* respiracao fora de fase entre os dois */
    double segundosCarga;
    Vector3 posicao;          /* calculada pela caixa */
    float escala;
} Personagem3D;

/* ----------------------------------------------------------------- shader */

/* Luz basica, GLSL 330 (OpenGL 3.3, o que a raylib usa no desktop, tambem no
   Windows). Lambert com duas luzes direcionais e ambiente; sem especular,
   porque a textura do Meshy ja traz luz e brilho "pintados". */
static const char *VERTICE =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "in vec3 vertexNormal;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matNormal;\n"
    "out vec2 fragTexCoord;\n"
    "out vec3 fragNormal;\n"
    "void main() {\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *FRAGMENTO =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec3 fragNormal;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform vec3 luzPrincipalDir;\n"     /* para onde a luz vai */
    "uniform vec3 luzPrincipalCor;\n"
    "uniform vec3 luzPreenchDir;\n"
    "uniform vec3 luzPreenchCor;\n"
    "uniform vec3 luzAmbiente;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec4 cor = texture(texture0, fragTexCoord) * colDiffuse;\n"
    /* Material doubleSided: a face de tras vista de frente (dentro do bone)
       usa a normal invertida, senao fica preta. */
    "    vec3 n = normalize(gl_FrontFacing ? fragNormal : -fragNormal);\n"
    "    vec3 luz = luzAmbiente\n"
    "             + luzPrincipalCor * max(dot(n, -luzPrincipalDir), 0.0)\n"
    "             + luzPreenchCor * max(dot(n, -luzPreenchDir), 0.0);\n"
    "    finalColor = vec4(cor.rgb * luz, cor.a);\n"
    "}\n";

static Shader carregar_shader(void) {
    Shader s = LoadShaderFromMemory(VERTICE, FRAGMENTO);
    /* Vindo da ESQUERDA (vai para +X), um pouco de cima e da frente. */
    Vector3 principal = Vector3Normalize((Vector3){ 1.0f, -0.45f, -0.55f });
    Vector3 corPrincipal = { 1.05f, 0.88f, 0.70f };   /* quente */
    /* Preenchimento fraco e frio pela DIREITA (vai para -X). */
    Vector3 preench = Vector3Normalize((Vector3){ -1.0f, -0.15f, -0.35f });
    Vector3 corPreench = { 0.22f, 0.27f, 0.36f };
    Vector3 ambiente = { 0.30f, 0.28f, 0.27f };

    s.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(s, "matNormal");
    SetShaderValue(s, GetShaderLocation(s, "luzPrincipalDir"), &principal, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "luzPrincipalCor"), &corPrincipal, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "luzPreenchDir"), &preench, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "luzPreenchCor"), &corPreench, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "luzAmbiente"), &ambiente, SHADER_UNIFORM_VEC3);
    return s;
}

/* ------------------------------------------------------------- personagens */

static Camera3D camera_da_cena(void) {
    Camera3D c = { 0 };
    c.position = (Vector3){ 0.0f, CAMERA_ALTURA, 0.0f };
    c.target = (Vector3){ 0.0f, CAMERA_ALTURA, -1.0f };   /* olha reto para -Z */
    c.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    c.fovy = FOV_GRAUS;
    c.projection = CAMERA_PERSPECTIVE;
    return c;
}

/* Quanto mede no mundo um pixel logico, a uma profundidade d. */
static float mundo_por_pixel(float d) {
    return 2.0f * d * tanf(FOV_GRAUS * 0.5f * DEG2RAD) / TELA_ALTURA;
}

/* Pixels logicos -> mundo: centro da caixa em X e pes na base da caixa.
   escalaMundo <= 0: este e o personagem de referencia (o parceiro), que
   define a escala do mundo pela altura da caixa dele. Os outros usam a
   escala de referencia vezes a relativa (p2 = 1,03) e ficam na profundidade
   em que essa altura ocupa a caixa deles: tamanho certo no mundo e lugar
   certo na tela. Devolve a escala de mundo da referencia. */
static float encaixar_na_caixa(Personagem3D *p, float escalaMundo) {
    float cx = p->caixa.x + p->caixa.width * 0.5f;
    float base = p->caixa.y + p->caixa.height;
    float mpp;

    if (escalaMundo <= 0.0f) {
        mpp = mundo_por_pixel(p->profundidade);
        escalaMundo = p->caixa.height * mpp / ALTURA_MODELO;
    }
    p->escala = escalaMundo * p->escalaRelativa;
    mpp = p->escala * ALTURA_MODELO / p->caixa.height;
    p->profundidade = mpp * TELA_ALTURA / (2.0f * tanf(FOV_GRAUS * 0.5f * DEG2RAD));
    p->posicao = (Vector3){ (cx - TELA_LARGURA * 0.5f) * mpp,
                            CAMERA_ALTURA - (base - TELA_ALTURA * 0.5f) * mpp,
                            -p->profundidade };
    return escalaMundo;
}

static void carregar_personagem(Personagem3D *p, const char *arquivo, Shader shader) {
    const char *caminho = TextFormat("%s../assets/modelos/%s", GetApplicationDirectory(), arquivo);
    double inicio = GetTime();

    p->modelo = LoadModel(caminho);
    /* A raylib poe um material padrao no indice 0 e os do arquivo depois:
       o que vale e o material que a malha usa (meshMaterial). Uma malha so,
       um material so (preparar_modelo.py confere). */
    {
        Material *m = &p->modelo.materials[p->modelo.meshMaterial[0]];
        Texture2D *t = &m->maps[MATERIAL_MAP_DIFFUSE].texture;
        /* Mipmaps + trilinear: o modelo aparece bem menor que a textura de
           2048, e sem mipmaps a textura "chia" quando ele respira. */
        GenTextureMipmaps(t);
        SetTextureFilter(*t, TEXTURE_FILTER_TRILINEAR);
        m->shader = shader;
    }
    p->segundosCarga = GetTime() - inicio;
}

static void desenhar_personagem3d(const Personagem3D *p, double t) {
    float respira = 1.0f + RESPIRACAO_AMPLITUDE *
                    sinf((float)(t * 2.0 * PI / RESPIRACAO_PERIODO) + p->fase);
    DrawModelEx(p->modelo, p->posicao, (Vector3){ 0, 1, 0 }, p->giroY,
                (Vector3){ p->escala, p->escala * respira, p->escala }, WHITE);
}

/* ------------------------------------------------------------------ cena */

static RenderTexture2D alvo3d;
static int descarteLigado = 1;

static void desenhar_3d(Personagem3D *ps, int qtd, double t) {
    Camera3D camera = camera_da_cena();
    BeginTextureMode(alvo3d);
    ClearBackground(BLANK);
    BeginMode3D(camera);
    /* O material e doubleSided e a raylib descarta as faces de tras: sem
       desligar, abas finas (bone, laco do lenco) ficam vazadas por dentro. */
    if (!descarteLigado) rlDisableBackfaceCulling();
    /* Do mais longe para o mais perto (o parceiro antes do vizinho). */
    for (int i = 0; i < qtd; i++) desenhar_personagem3d(&ps[i], t);
    rlEnableBackfaceCulling();
    EndMode3D();
    EndTextureMode();
}

static void desenhar_cena(void) {
    HudDados hud = { 0 };
    Rectangle origem = { 0, 0, (float)alvo3d.texture.width, -(float)alvo3d.texture.height };

    hud.valorMao = 1;
    hud.resultados[0] = hud.resultados[1] = hud.resultados[2] = INDEFINIDO;
    hud.vira = (Carta){ COPAS, SEIS };
    snprintf(hud.rotuloPedir, sizeof hud.rotuloPedir, "PEDIR TRUCO");

    desenhar_panorama();
    desenhar_paineis();
    desenhar_personagem(0, POSICAO_ESQUERDA);   /* o da esquerda continua 2D */
    DrawTexturePro(alvo3d.texture, origem, (Rectangle){ 0, 0, TELA_LARGURA, TELA_ALTURA },
                   (Vector2){ 0, 0 }, 0.0f, WHITE);
    desenhar_mesa();
    desenho_versos_dos_outros(3, 3, 3);
    desenho_monte();
    desenho_vira(hud.vira);
    desenho_mao_versos(3);
    hud_desenhar(&hud, 0);
}

/* ------------------------------------------------------- medicao e captura */

static void capturar(const char *pasta, const char *nome, const Rectangle *zoom) {
    char caminho[512];
    Image img;
    snprintf(caminho, sizeof caminho, "%s/%s", pasta, nome);
    TakeScreenshot(caminho);
    img = LoadImage(caminho);
    /* Retina: a raylib 6.2 grava o dobro, com a tela no quadrante de baixo
       a esquerda. */
    if (img.width >= 2 * GetScreenWidth())
        ImageCrop(&img, (Rectangle){ 0, img.height / 2.0f, img.width / 2.0f, img.height / 2.0f });
    if (zoom) {
        float k = img.width / (float)TELA_LARGURA;
        ImageCrop(&img, (Rectangle){ zoom->x * k, zoom->y * k, zoom->width * k, zoom->height * k });
    }
    ExportImage(img, caminho);
    printf("CAPTURA %s %dx%d\n", caminho, img.width, img.height);
    UnloadImage(img);
}

/* Quantos pixels mudam na imagem 3D entre descarte ligado e desligado:
   se a diferenca for grande, ha faces de tras visiveis (buracos). */
static int pixels_diferentes(Personagem3D *ps, int qtd, Rectangle regiao) {
    Image com, sem;
    int diferentes = 0;
    float k = alvo3d.texture.width / (float)TELA_LARGURA;

    descarteLigado = 1; desenhar_3d(ps, qtd, 0.0);
    com = LoadImageFromTexture(alvo3d.texture);
    descarteLigado = 0; desenhar_3d(ps, qtd, 0.0);
    sem = LoadImageFromTexture(alvo3d.texture);
    for (int y = (int)(regiao.y * k); y < (int)((regiao.y + regiao.height) * k); y++) {
        for (int x = (int)(regiao.x * k); x < (int)((regiao.x + regiao.width) * k); x++) {
            /* A imagem da RenderTexture vem de cabeca para baixo. */
            Color a = GetImageColor(com, x, com.height - 1 - y);
            Color b = GetImageColor(sem, x, sem.height - 1 - y);
            if (abs(a.r - b.r) + abs(a.g - b.g) + abs(a.b - b.b) + abs(a.a - b.a) > 24) diferentes++;
        }
    }
    UnloadImage(com);
    UnloadImage(sem);
    return diferentes;
}

static void quadro(Personagem3D *ps, int qtd) {
    desenhar_3d(ps, qtd, GetTime());
    tela_comecar_desenho();
    desenhar_cena();
    tela_terminar_desenho();
}

static void medir(Personagem3D *ps, int qtd, const char *pasta) {
    double inicio;
    int quadros = 0;
    long memoria = 0;
    Rectangle zoomFrente = { 700, 140, 520, 470 };
    Rectangle zoomDireita = { 1300, 260, 620, 560 };

    for (int i = 0; i < qtd; i++) {
        Texture2D t = ps[i].modelo.materials[ps[i].modelo.meshMaterial[0]].maps[MATERIAL_MAP_DIFFUSE].texture;
        long base = (long)t.width * t.height * 4;
        memoria += base * 4 / 3;   /* mipmaps somam 1/3 */
        printf("MODELO %d: %d triangulos, textura %dx%d, %d mipmaps, %.0f ms de carga, %.1f MB de textura com mipmaps\n",
               i + 1, ps[i].modelo.meshes[0].triangleCount, t.width, t.height, t.mipmaps,
               ps[i].segundosCarga * 1000.0, base * 4.0 / 3.0 / 1e6);
    }
    printf("TEXTURA TOTAL: %.1f MB | RenderTexture 3D %dx%d: %.1f MB (cor + profundidade)\n", memoria / 1e6,
           alvo3d.texture.width, alvo3d.texture.height, alvo3d.texture.width * (double)alvo3d.texture.height * 8 / 1e6);

    /* aquece */
    for (int i = 0; i < 60; i++) quadro(ps, qtd);
    inicio = GetTime();
    while (GetTime() - inicio < 10.0) { quadro(ps, qtd); quadros++; }
    printf("FPS MEDIO (sem vsync, 10 s, janela %dx%d px, 3D em %dx%d): %.0f (%.2f ms por quadro)\n",
           GetRenderWidth(), GetRenderHeight(), alvo3d.texture.width, alvo3d.texture.height,
           quadros / (GetTime() - inicio), (GetTime() - inicio) * 1000.0 / quadros);

    printf("DESCARTE: pixels que mudam ao desligar o descarte de faces de tras: frente %d, direita %d\n",
           pixels_diferentes(ps, qtd, zoomFrente), pixels_diferentes(ps, qtd, zoomDireita));

    for (int k = 0; k < 2; k++) {
        descarteLigado = (k == 0);
        for (int i = 0; i < 4; i++) quadro(ps, qtd);   /* o print le o quadro anterior */
        capturar(pasta, descarteLigado ? "cena_descarte_ligado.png" : "cena_descarte_desligado.png", NULL);
        capturar(pasta, descarteLigado ? "zoom_p1_frente_descarte_ligado.png" : "zoom_p1_frente_descarte_desligado.png",
                 &zoomFrente);
        capturar(pasta, descarteLigado ? "zoom_p2_direita_descarte_ligado.png" : "zoom_p2_direita_descarte_desligado.png",
                 &zoomDireita);
    }
}

int main(int argc, char **argv) {
    int medindo = (argc >= 3 && strcmp(argv[1], "--medir") == 0);
    Personagem3D ps[2] = {
        /* parceiro (frente) primeiro: e o mais longe */
        { .caixa = CENA_PERSONAGEM_FRENTE, .profundidade = PROF_FRENTE, .escalaRelativa = 1.00f,
          .giroY = 0.0f, .fase = 0.0f },
        /* direita: virado para o centro da mesa (o modelo olha para +Z,
           girar -70 graus o deixa quase de perfil, olhando para a esquerda) */
        { .caixa = CENA_PERSONAGEM_DIREITA, .escalaRelativa = 1.03f,
          .giroY = -70.0f, .fase = 2.1f },
    };
    Shader shader;
    double inicioCarga;

    /* Medindo, sem VSYNC: o FPS medido e o que a maquina aguenta. */
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | (medindo ? 0 : FLAG_VSYNC_HINT));
    InitWindow(1600, 900, "Experimento 3D - Truco no Bar do Pato");
    SetExitKey(KEY_ESCAPE);
    tela_iniciar();
    recursos_carregar();

    alvo3d = LoadRenderTexture(TELA_LARGURA * SUPERAMOSTRAGEM, TELA_ALTURA * SUPERAMOSTRAGEM);
    SetTextureFilter(alvo3d.texture, TEXTURE_FILTER_BILINEAR);
    shader = carregar_shader();
    inicioCarga = GetTime();
    carregar_personagem(&ps[0], "p1.glb", shader);
    carregar_personagem(&ps[1], "p2.glb", shader);
    printf("CARGA TOTAL: %.0f ms\n", (GetTime() - inicioCarga) * 1000.0);
    {
        float escalaMundo = encaixar_na_caixa(&ps[0], 0.0f);
        encaixar_na_caixa(&ps[1], escalaMundo);
        for (int i = 0; i < 2; i++)
            printf("LUGAR p%d: escala %.3f (relativa %.2f), profundidade %.2f, pes em (%.2f, %.2f)\n", i + 1,
                   ps[i].escala, ps[i].escalaRelativa, ps[i].profundidade, ps[i].posicao.x, ps[i].posicao.y);
    }

    if (medindo) {
        medir(ps, 2, argv[2]);
    } else {
        while (!WindowShouldClose()) {
            if (IsKeyPressed(KEY_C)) descarteLigado = !descarteLigado;
            quadro(ps, 2);
        }
    }

    for (int i = 0; i < 2; i++) UnloadModel(ps[i].modelo);
    UnloadShader(shader);
    UnloadRenderTexture(alvo3d);
    recursos_descarregar();
    tela_encerrar();
    CloseWindow();
    return 0;
}
