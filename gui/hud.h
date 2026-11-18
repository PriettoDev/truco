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

/* Resposta ao truco: tres botoes empilhados na mesma coluna, o de baixo no
   lugar do PEDIR TRUCO (que some na resposta). Os versos da direita ficam
   mais para o meio para a coluna caber (desenho_cartas.h). */
#define HUD_BOTAO_ACEITAR  ((Rectangle){ 1580, 686, 316,  86 })
#define HUD_BOTAO_AUMENTAR ((Rectangle){ 1580, 786, 316,  86 })
#define HUD_BOTAO_CORRER   ((Rectangle){ 1580, 886, 316,  86 })

/* Cores de "Nos", "Eles" e empate, do ponto de vista de quem esta vendo.
   Ficam aqui porque as telas de apuracao usam as mesmas do placar. */
#define HUD_COR_NOS    ((Color){  86, 186, 110, 255 })
#define HUD_COR_ELES   ((Color){ 222,  88,  72, 255 })
#define HUD_COR_EMPATE ((Color){ 170, 170, 170, 255 })

typedef enum { HUD_NENHUMA, HUD_PEDIR_TRUCO, HUD_VAR, HUD_ACEITAR, HUD_AUMENTAR, HUD_CORRER } HudAcao;

/* Tudo o que o HUD mostra. Duplas na numeracao interna (0 e 1), como em
   rodada/mao/aposta; o HUD converte para "Nos" e "Eles" pelo assento. */
typedef struct {
    int pontos[2];        /* pontos de cada dupla interna */
    int valorMao;         /* aposta.valor confirmado */
    int resultados[3];    /* Mao.resultados: 0, 1, EMPATE ou INDEFINIDO */
    Carta vira;
    int valorPedido;      /* pedido pendente (0 = nenhum): "VALE 1 -> 3?" */

    /* Botoes. Rotulos prontos (vem de partida_nome_do_valor), para o HUD
       nao conhecer a sequencia de valores. Desativado = cinza, sem clique. */
    int resposta;         /* 1: ACEITAR / PEDIR MAIS / CORRER no lugar do PEDIR TRUCO */
    int pedirAtivo;       /* PEDIR TRUCO na vez, ou PEDIR MAIS na resposta */
    char rotuloPedir[24]; /* "PEDIR TRUCO", "PEDIR SEIS", "PEDIR 15"... */
    int respostaAtiva;    /* ACEITAR e CORRER (depois da trava) */
    int varAtivo;         /* VAR so na Etapa 4 */
} HudDados;

/* Anima o destaque dos botoes e devolve o botao clicado neste quadro.
   Botao desativado nao destaca e nunca devolve clique. */
HudAcao hud_atualizar(const HudDados *dados, Vector2 mouse, float dt);

/* O mouse esta sobre um botao ativo (para a maozinha no cursor; quem muda
   o cursor e o main_gui, que tambem olha a mao do jogador). */
int hud_mouse_sobre_botao(void);

void hud_desenhar(const HudDados *dados, int assentoVisto);

#endif
