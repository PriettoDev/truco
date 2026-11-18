#include "conferencia.h"
#include "cena.h"
#include "desenho_cartas.h"
#include "hud.h"
#include "telas_partida.h"

typedef struct {
    Rectangle area;
    const char *nome;
} Area;

#define QTD(v) ((int)(sizeof(v) / sizeof((v)[0])))

/* Compara cada area de "quem" com cada area de "contra" e avisa as que se
   cruzam. Devolve quantas sobreposicoes achou. */
static int cruzar(const Area quem[], int qtdQuem, const Area contra[], int qtdContra,
                  const char *verbo) {
    int achou = 0;
    for (int i = 0; i < qtdQuem; i++) {
        for (int j = 0; j < qtdContra; j++) {
            if (CheckCollisionRecs(quem[i].area, contra[j].area)) {
                TraceLog(LOG_WARNING, "CONFERENCIA: %s %s %s (ajuste as constantes)",
                         quem[i].nome, verbo, contra[j].nome);
                achou++;
            }
        }
    }
    return achou;
}

int conferir_posicoes(void) {
    const Area paineis[] = {
        { CENA_PAINEL_LOUSA, "painel lousa" },
        { CENA_PAINEL_MURAL, "painel mural" },
        { CENA_PAINEL_TV_VAR, "painel tv_var" },
        { CENA_PAINEL_CADERNETA, "painel caderneta" },
    };
    /* A caixa inteira do personagem, e nao so a imagem: a arte pode ocupar
       a caixa toda, e o painel tem que continuar clicavel. */
    const Area corposEMesa[] = {
        { CENA_PERSONAGEM_ESQUERDA, "personagem da esquerda" },
        { CENA_PERSONAGEM_FRENTE, "personagem da frente" },
        { CENA_PERSONAGEM_DIREITA, "personagem da direita" },
        { CENA_MESA, "mesa" },
    };
    const Area hud[] = {
        { HUD_PLACAR, "HUD placar" },
        { HUD_CARTAS_FORTES, "HUD cartas mais fortes" },
        { HUD_BOTAO_TRUCO, "botao PEDIR TRUCO" },
        { HUD_BOTAO_VAR, "botao VAR" },
        { HUD_BOTAO_ACEITAR, "botao ACEITAR" },
        { HUD_BOTAO_AUMENTAR, "botao PEDIR MAIS" },
        { HUD_BOTAO_CORRER, "botao CORRER" },
        { TELAS_RESULTADO_RODADA, "plaquinha da rodada / pedido" },
    };
    const Area protegidas[] = {
        { desenho_area_mao(3), "mao do jogador" },
        { desenho_area_vira_e_monte(), "vira e monte" },
        { desenho_area_cartas_jogadas(), "cartas jogadas" },
        { desenho_area_versos_lado(DESENHO_VERSOS_ESQUERDA_CENTRO), "versos da esquerda" },
        { desenho_area_versos_lado(DESENHO_VERSOS_DIREITA_CENTRO), "versos da direita" },
        { CENA_ROSTO_ESQUERDA, "rosto da esquerda" },
        { CENA_ROSTO_FRENTE, "rosto da frente" },
        { CENA_ROSTO_DIREITA, "rosto da direita" },
        { CENA_PAINEL_LOUSA, "painel lousa" },
        { CENA_PAINEL_MURAL, "painel mural" },
        { CENA_PAINEL_TV_VAR, "painel tv_var" },
        { CENA_PAINEL_CADERNETA, "painel caderneta" },
    };
    /* As cartas jogadas tambem nao podem encostar na vira/monte nem nos versos
       do parceiro, que ficam logo acima. */
    const Area cruz[] = { { desenho_area_cartas_jogadas(), "cartas jogadas" } };
    const Area vizinhosDaCruz[] = {
        { desenho_area_vira_e_monte(), "vira e monte" },
        { desenho_area_versos_lado(DESENHO_VERSOS_DIREITA_CENTRO), "versos da direita" },
        { desenho_area_versos_frente(), "versos do parceiro" },
    };
    int achou = 0;

    achou += cruzar(paineis, QTD(paineis), corposEMesa, QTD(corposEMesa), "fica atras do");
    achou += cruzar(hud, QTD(hud), protegidas, QTD(protegidas), "cobre");
    achou += cruzar(cruz, QTD(cruz), vizinhosDaCruz, QTD(vizinhosDaCruz), "encosta em");
    {   /* os versos da direita chegaram perto do monte quando abriram espaco
           para os botoes de resposta */
        const Area direita[] = { { desenho_area_versos_lado(DESENHO_VERSOS_DIREITA_CENTRO), "versos da direita" } };
        const Area monte[] = { { desenho_area_vira_e_monte(), "vira e monte" } };
        achou += cruzar(direita, QTD(direita), monte, QTD(monte), "encosta em");
    }
    if (achou == 0) TraceLog(LOG_INFO, "CONFERENCIA: nenhuma area de tela se sobrepoe");
    return achou == 0;
}
