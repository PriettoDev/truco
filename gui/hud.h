#ifndef HUD_H
#define HUD_H

/*
 * hud.h - camada 6, a interface por cima de tudo: placar, painel de cartas
 * mais fortes e botoes. So le os dados que recebe; quem decide o que fazer
 * com um clique e o main_gui.
 *
 * REGRA: nada aqui pode cobrir a mao, a vira/monte, o rosto dos personagens
 * nem os paineis do bar. conferir_posicoes() (conferencia.c) verifica.
 */
#include "raylib.h"
#include "carta.h"

/* Posicoes provisorias, entre os paineis do bar e acima dos rostos. */
#define HUD_PLACAR         ((Rectangle){  745,  14, 400, 146 })   /* entre mural e tv */
#define HUD_CARTAS_FORTES  ((Rectangle){ 1470,  14, 436, 222 })   /* acima do rosto da direita */
#define HUD_BOTAO_TRUCO    ((Rectangle){ 1580, 886, 316,  86 })
#define HUD_BOTAO_VAR      ((Rectangle){ 1580, 986, 316,  70 })

typedef enum { HUD_NENHUMA, HUD_PEDIR_TRUCO, HUD_VAR } HudAcao;

/* Tudo o que o HUD mostra. Duplas na numeracao interna (0 e 1), como em
   rodada/mao/aposta; o HUD converte para "Nos" e "Eles" pelo assento. */
typedef struct {
    int pontos[2];        /* pontos de cada dupla interna */
    int valorMao;         /* aposta.valor confirmado */
    int resultados[3];    /* Mao.resultados: 0, 1, EMPATE ou INDEFINIDO */
    Carta vira;
} HudDados;

/* Anima o destaque dos botoes e devolve o botao clicado neste quadro.
   Com botoesAtivos = 0 (truco e VAR ainda nao existem) os botoes nao
   destacam, nao mudam o cursor e nunca devolvem clique. */
HudAcao hud_atualizar(Vector2 mouse, float dt, int botoesAtivos);

void hud_desenhar(const HudDados *dados, int assentoVisto, int botoesAtivos);

#endif
