#include <stdio.h>
#include <stdlib.h>   // srand
#include <time.h>     // time
#include "mesa.h"
#include "mao.h"
#include "baralho.h"

/*
 * VOCABULARIO USADO NESTE ARQUIVO
 *   mao     = cada vez que os 4 jogadores jogam uma carta (Rodada no codigo do Breno)
 *   rodada  = conjunto de ate 3 maos, que vale pontos (Mao no mao.h)
 *   jogo    = varias rodadas, ate uma dupla chegar em PONTOS_PARA_VENCER
 */

/* ------------------------------------------------------------------ */
/* PREPARACAO DO JOGO                                                  */
/* ------------------------------------------------------------------ */

// Cria a mesa, os 4 jogadores e coloca todos na lista circular.
// Retorna 1 se deu certo, 0 se houve erro.
static int configurarMesa(Mesa *mesa) {
    if (!inicializarMesa(mesa)) {
        printf("Erro ao inicializar a mesa.\n");
        return 0;
    }

    // Posicoes 0 e 2 formam a dupla 1; posicoes 1 e 3 formam a dupla 2
    mesa->jogadores[0] = jogador_criar("Lucas", 1, 0);
    mesa->jogadores[1] = jogador_criar("Ana", 2, 1);
    mesa->jogadores[2] = jogador_criar("Pedro", 1, 2);
    mesa->jogadores[3] = jogador_criar("Maria", 2, 3);

    for (int i = 0; i < MAX_JOGADOR; i++) {
        if (!inserirJogadorNaOrdem(mesa, i)) {
            printf("Erro ao inserir jogador.\n");
            destruirMesa(mesa);
            return 0;
        }
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* INICIO DE UMA RODADA                                                */
/* ------------------------------------------------------------------ */

// Embaralha, entrega 3 cartas para cada jogador e vira a carta.
// Guarda a manilha em *manilha. Retorna 1 se deu certo, 0 se houve erro.
static int distribuirCartas(Mesa *mesa, Valor *manilha) {
    Baralho baralho;
    Carta vira;

    inicializaBaralho(&baralho);
    criaBaralho(&baralho);
    embaralhaBaralho(&baralho);

    for (int i = 0; i < MAX_JOGADOR; i++) {
        Carta c1, c2, c3;

        if (!retiraCarta(&baralho, &c1) ||
            !retiraCarta(&baralho, &c2) ||
            !retiraCarta(&baralho, &c3)) {
            printf("Erro ao distribuir as cartas.\n");
            return 0;
        }

        jogador_distribuir_cartas(&mesa->jogadores[i], c1, c2, c3);
    }

    if (!retiraCarta(&baralho, &vira)) {
        printf("Erro ao virar a carta.\n");
        return 0;
    }

    *manilha = manilha_da_vira(vira);

    printf("Vira: ");
    carta_imprimir(vira);
    printf("  |  Manilha: %s\n", valor_texto(*manilha));

    return 1;
}

/* ------------------------------------------------------------------ */
/* UMA MAO (os 4 jogadores jogam uma carta)                            */
/* ------------------------------------------------------------------ */

// Cada jogador, na ordem da lista, joga uma carta.
static void jogarCartas(Mesa *mesa, Mao *mao) {
    for (tp_no *no = mesa->ordem->ini; no != NULL; no = no->prox) {
        int j = no->info;   // posicao do jogador (0 a 3)

        // Jogada automatica de teste: na mao N, joga a carta N da mao dele.
        // Para jogo real, troque esta linha pela escolha do jogador.
        Carta carta = mesa->jogadores[j].mao[mao->rodada_atual];

        printf("%s joga: ", mesa->jogadores[j].nome);
        carta_imprimir(carta);
        printf("\n");

        // j % 2 = dupla 0 ou 1 (o Mao e o Aposta usam 0 e 1)
        if (!mao_jogar(mao, carta, j, j % 2)) {
            printf("Jogada invalida do jogador %s.\n", mesa->jogadores[j].nome);
        }
    }
}

// Mostra quem ganhou a mao que acabou de ser apurada.
static void mostrarResultadoDaMao(const Mao *mao, int n) {
    if (mao->resultados[n] == EMPATE) {
        printf("Mao %d empatou.\n", n + 1);
    } else {
        printf("Mao %d: dupla %d venceu.\n", n + 1, mao->resultados[n] + 1);
    }
}

// Define quem comeca a proxima mao:
//   - empate: a mesa gira um passo
//   - vitoria: quem ganhou a mao comeca a proxima
static void definirQuemAbreProximaMao(Mesa *mesa, const Mao *mao, int n) {
    if (mao->resultados[n] == EMPATE) {
        prepararProximaMao(mesa, EMPATE_MAO);
    } else {
        prepararProximaMao(mesa, mao->jogador_abre);
    }
}

/* ------------------------------------------------------------------ */
/* UMA RODADA (ate 3 maos)                                             */
/* ------------------------------------------------------------------ */

// Soma os pontos da rodada na dupla vencedora (empate nao pontua).
static void somarPontos(Mesa *mesa, const Mao *mao) {
    if (mao->dupla_vencedora == 0) {
        mesa->pontosDupla1 += mao->pontos_ganhos;
    } else if (mao->dupla_vencedora == 1) {
        mesa->pontosDupla2 += mao->pontos_ganhos;
    }
}

static void mostrarPlacar(const Mesa *mesa) {
    printf("\nPlacar: Dupla 1 = %d | Dupla 2 = %d\n",
           mesa->pontosDupla1, mesa->pontosDupla2);
}

// Joga uma rodada completa. Retorna 1 se deu certo, 0 se houve erro.
static int jogarRodada(Mesa *mesa) {
    Mao mao;
    Valor manilha;

    // Quem abre: o proximo jogador em relacao a quem abriu a rodada anterior
    int abre = iniciarNovaRodada(mesa);
    if (abre < 0) {
        printf("Erro ao definir quem abre a rodada.\n");
        return 0;
    }

    printf("\n===== RODADA %d =====\n", mesa->numeroDaRodada);

    if (!distribuirCartas(mesa, &manilha)) {
        return 0;
    }

    mao_iniciar(&mao, manilha, abre);

    // Cada volta e uma mao, ate a rodada terminar
    while (!mao_terminou(&mao)) {
        printf("\n--- MAO %d ---", mao.rodada_atual + 1);
        imprimirOrdemDaMao(mesa);

        jogarCartas(mesa, &mao);

        if (!mao_apurar_rodada(&mao)) {
            printf("Nao foi possivel apurar a mao.\n");
            return 0;
        }

        int n = mao.rodada_atual - 1;   // a mao que acabou de ser apurada
        mostrarResultadoDaMao(&mao, n);

        if (!mao_terminou(&mao)) {
            definirQuemAbreProximaMao(mesa, &mao, n);
        }
    }

    somarPontos(mesa, &mao);
    mostrarPlacar(mesa);
    mesa->numeroDaRodada++;

    return 1;
}

/* ------------------------------------------------------------------ */
/* PROGRAMA PRINCIPAL                                                  */
/* ------------------------------------------------------------------ */

int main(void) {
    Mesa mesa;

    srand((unsigned) time(NULL));   // para o embaralhamento mudar a cada jogo

    if (!configurarMesa(&mesa)) {
        return 1;
    }

    // Joga rodadas ate uma dupla chegar em PONTOS_PARA_VENCER
    while (mesa.pontosDupla1 < PONTOS_PARA_VENCER &&
           mesa.pontosDupla2 < PONTOS_PARA_VENCER) {

        if (!jogarRodada(&mesa)) {
            destruirMesa(&mesa);
            return 1;
        }
    }

    if (mesa.pontosDupla1 >= PONTOS_PARA_VENCER) {
        printf("\nDupla 1 venceu o jogo!\n");
    } else {
        printf("\nDupla 2 venceu o jogo!\n");
    }

    destruirMesa(&mesa);
    return 0;
}
