/*
 * partida_gui.c - maquina de estados da partida no modo passa a vez.
 *
 * Reproduz o fluxo do src/main.c (as linhas citadas sao dele), chamando os
 * mesmos modulos, mas sem ler o teclado: quem joga avisa pela tela com
 * partida_confirmar e partida_jogar_carta, e o tempo passa com
 * partida_atualizar. Nada de raylib aqui, para o teste rodar sem janela.
 *
 *            +-------------+
 *            | INICIO_MAO  |  embaralha, distribui, vira (automatico)
 *            +------+------+
 *                   v
 *            +-------------+  clique/ESPACO
 *   +------->|  COBERTURA  |----------------+   (so depois de 0,3 s)
 *   |        +-------------+                v
 *   |                              +----------------+
 *   |                              | VEZ_DO_JOGADOR |
 *   |                              +-------+--------+
 *   |                                      | clicou numa carta
 *   |                                      v
 *   |   faltam cartas na rodada    +----------------+
 *   +------------------------------|  CARTA_VOANDO  |  0,35 s
 *   |                              +-------+--------+
 *   |                                      | 4 cartas na mesa
 *   |   a mao continua             +-------v--------+
 *   +------------------------------|    APURANDO    |  1,5 s
 *                                  +-------+--------+
 *                                          | mao terminou
 *                                  +-------v--------+  clique
 *        INICIO_MAO <--------------|    FIM_MAO     |-----------+
 *        (ninguem chegou a 12)     +----------------+           | uma dupla fez 12
 *                                                               v
 *                                  +----------------+  clique   +-------------+
 *                                  |  nova partida  |<----------| FIM_PARTIDA |
 *                                  +----------------+           +-------------+
 *
 * Cliques e ESPACO sao ignorados em CARTA_VOANDO e APURANDO. Nas telas que
 * esperam clique (COBERTURA, FIM_MAO, FIM_PARTIDA) e na escolha da carta,
 * o clique so vale depois de PARTIDA_TEMPO_TRAVA, para um clique duplo nao
 * passar direto pela tela seguinte.
 */
#include <stdio.h>

#include "partida_gui.h"

/* Assento 0 a 3. Os pares (0 e 2) sao a dupla interna 0. */
static const char *NOMES[NUM_JOGADORES] = { "Seu Bigode", "Dona Marlene", "Tonhão", "Kiko" };

/* Toda funcao de src/ que pode falhar passa por aqui: o jogo segue, mas o
   aviso no terminal e o contador mostram que algo saiu da regra. */
static void falhou(PartidaGui *p, const char *oque) {
    fprintf(stderr, "PARTIDA: falhou %s (estado %d)\n", oque, (int)p->estado);
    p->falhas++;
}

static void mudar_estado(PartidaGui *p, EstadoPartida novo) {
    p->estado = novo;
    p->tempoNoEstado = 0.0f;
}

const char *partida_nome(int assento) {
    if (assento < 0 || assento >= NUM_JOGADORES) return "?";
    return NOMES[assento];
}

int partida_pontos(const PartidaGui *p, int dupla) {
    /* pontosDupla1 e a dupla interna 0 (assentos 0 e 2), como no somarPontos. */
    return dupla == 0 ? p->mesa.pontosDupla1 : p->mesa.pontosDupla2;
}

/* ------------------------------------------------------------ preparacao */

/* main.c 113-117 e 147-148: fila vazia, mao 1, ninguem abriu ainda e placar
   zerado; os jogadores entram na fila na ordem dos assentos. */
static void preparar_mesa(PartidaGui *p) {
    inicializarMesa(&p->mesa.ordem);
    p->mesa.numeroDaRodada = 1;
    p->mesa.inicioDaRodada = -1;
    p->mesa.pontosDupla1 = 0;
    p->mesa.pontosDupla2 = 0;

    for (int i = 0; i < NUM_JOGADORES; i++) {
        char nome[30];
        /* jogador_criar pede char[] (nao const): copia o nome antes. */
        snprintf(nome, sizeof(nome), "%s", NOMES[i]);
        p->mesa.jogadores[i] = jogador_criar(nome, i % 2 + 1, i);
        if (!inserirMesa(&p->mesa.ordem, &p->mesa.jogadores[i])) falhou(p, "inserirMesa");
    }
}

void partida_iniciar(PartidaGui *p) {
    /* Zera por atribuicao de um literal (nao e copia de outra partida). */
    *p = (PartidaGui){ 0 };
    p->destaque[0] = p->destaque[1] = -1;
    p->resultadoRodada = INDEFINIDO;
    p->duplaVencedoraMao = MAO_SEM_VENCEDOR;
    p->duplaVencedoraPartida = -1;
    preparar_mesa(p);
    mudar_estado(p, PARTIDA_INICIO_MAO);
}

/* main.c 160-187 (distribuirCartas): baralho novo embaralhado, 3 cartas
   seguidas para cada um na ordem da fila (comecando por quem abre) e so
   depois a vira. A fila e copiada para a ordem real nao mudar. */
static void distribuir(PartidaGui *p) {
    Ordem ordem = p->mesa.ordem;
    Jogador *jogador;
    Carta c1, c2, c3;

    inicializaBaralho(&p->baralho);
    criaBaralho(&p->baralho);
    embaralhaBaralho(&p->baralho);

    while (removerMesa(&ordem, &jogador)) {
        if (!retiraCarta(&p->baralho, &c1) || !retiraCarta(&p->baralho, &c2) ||
            !retiraCarta(&p->baralho, &c3)) {
            falhou(p, "retiraCarta");
            return;
        }
        jogador_distribuir_cartas(jogador, c1, c2, c3);
    }
    if (!retiraCarta(&p->baralho, &p->mesa.vira)) falhou(p, "retiraCarta (vira)");
    p->manilha = manilha_da_vira(p->mesa.vira);
}

/* ------------------------------------------------------------- rodadas */

/* main.c 373: o proximo da copia da fila e quem joga agora. */
static void chamar_proximo(PartidaGui *p) {
    Jogador *jogador;
    if (!removerMesa(&p->fila, &jogador)) {
        falhou(p, "removerMesa (fila da rodada)");
        return;
    }
    p->assentoDaVez = jogador_assento(jogador);
    mudar_estado(p, PARTIDA_COBERTURA);
}

/* main.c 361: cada rodada consome uma copia nova de mesa.ordem. */
static void comecar_rodada(PartidaGui *p) {
    p->fila = p->mesa.ordem;
    p->qtdNaMesa = 0;
    p->destaque[0] = p->destaque[1] = -1;
    p->resultadoRodada = INDEFINIDO;
    chamar_proximo(p);
}

/* main.c 450-458 (jogarMao): iniciarNovaRodada decide quem abre (assento 0
   na primeira mao, depois o proximo da roda), distribui e inicia a Mao. */
static void comecar_mao(PartidaGui *p) {
    int abre = iniciarNovaRodada(&p->mesa);
    if (abre < 0) {
        falhou(p, "iniciarNovaRodada");
        abre = 0;
    }
    distribuir(p);
    mao_iniciar(&p->mao, p->manilha, abre);
    p->duplaVencedoraMao = MAO_SEM_VENCEDOR;
    p->pontosDaMao = 0;
    comecar_rodada(p);
}

static int indice_na_mesa(const PartidaGui *p, int assento) {
    for (int i = 0; i < p->qtdNaMesa; i++) {
        if (p->naMesa[i].assento == assento) return i;
    }
    return -1;
}

/* Quais cartas acender. Na vitoria, a do jogador_vencedor que o proprio
   modulo apontou. No empate, a melhor de CADA dupla com a mesma regra da
   fase 1 do rodada_apurar (carta_comparar > 0 estrito: entre cartas iguais
   da mesma dupla fica a que foi jogada primeiro). Parceiros com cartas
   iguais nao sao empate: o empate so existe entre as duplas. */
static void marcar_destaque(PartidaGui *p, const Rodada *r) {
    p->destaque[0] = p->destaque[1] = -1;

    if (r->dupla_vencedora == 0 || r->dupla_vencedora == 1) {
        p->destaque[0] = indice_na_mesa(p, r->jogador_vencedor);
    } else if (r->dupla_vencedora == EMPATE) {
        int melhor[2] = { -1, -1 };
        for (int i = 0; i < r->qtd; i++) {
            int d = r->jogadas[i].dupla;
            if (melhor[d] == -1 ||
                carta_comparar(r->jogadas[i].carta, r->jogadas[melhor[d]].carta, p->mao.manilha) > 0) {
                melhor[d] = i;
            }
        }
        for (int d = 0; d < 2; d++) {
            if (melhor[d] >= 0) p->destaque[d] = indice_na_mesa(p, r->jogadas[melhor[d]].jogador);
        }
    }
    if (p->destaque[0] < 0) falhou(p, "achar a carta vencedora na mesa");
}

/* main.c 418: apura a rodada que acabou de completar. */
static void apurar(PartidaGui *p) {
    int n = p->mao.rodada_atual;
    if (!mao_apurar_rodada(&p->mao)) {
        falhou(p, "mao_apurar_rodada");
        return;
    }
    p->resultadoRodada = p->mao.resultados[n];
    marcar_destaque(p, &p->mao.rodadas[n]);
    mudar_estado(p, PARTIDA_APURANDO);
}

/* TEMPORARIO: substituir pela funcao publica do modulo de partida (Pessoa 1).
   Copia do somarPontos (static) do main.c 439-445: soma os pontos da mao a
   dupla vencedora e limita a PONTOS_PARA_VENCER; mao empatada nao pontua. */
static void somarPontos(Mesa *mesa, const Mao *mao) {
    int *placar;
    if (mao->dupla_vencedora != 0 && mao->dupla_vencedora != 1) return;
    placar = mao->dupla_vencedora == 0 ? &mesa->pontosDupla1 : &mesa->pontosDupla2;
    *placar += mao->pontos_ganhos;
    if (*placar > PONTOS_PARA_VENCER) *placar = PONTOS_PARA_VENCER;
}

/* main.c 465-472: guarda o resultado para a tela, soma e passa a mao. */
static void encerrar_mao(PartidaGui *p) {
    p->duplaVencedoraMao = p->mao.dupla_vencedora;
    p->pontosDaMao = p->mao.pontos_ganhos;
    somarPontos(&p->mesa, &p->mao);
    p->mesa.numeroDaRodada++;
    mudar_estado(p, PARTIDA_FIM_MAO);
}

/* ------------------------------------------------------ entradas da tela */

int partida_jogar_carta(PartidaGui *p, int pos) {
    Jogador *jogador;
    Carta carta, removida;
    int assento = p->assentoDaVez;

    if (p->estado != PARTIDA_VEZ_DO_JOGADOR || p->tempoNoEstado < PARTIDA_TEMPO_TRAVA) return 0;
    jogador = &p->mesa.jogadores[assento];
    if (pos < 0 || pos >= jogador->qtdCartas) return 0;

    /* main.c 347-350: registra na rodada ANTES de tirar da mao, para nao
       perder a carta se a regra recusar. A dupla interna e assento % 2. */
    carta = jogador->mao[pos];
    if (!mao_jogar(&p->mao, carta, assento, assento % 2)) {
        falhou(p, "mao_jogar");
        return 0;
    }
    if (!jogador_jogar_carta(jogador, pos, &removida)) {
        falhou(p, "jogador_jogar_carta");
        return 0;
    }
    p->naMesa[p->qtdNaMesa].carta = removida;
    p->naMesa[p->qtdNaMesa].assento = assento;
    p->naMesa[p->qtdNaMesa].posicaoNaMao = pos;
    p->qtdNaMesa++;
    mudar_estado(p, PARTIDA_CARTA_VOANDO);
    return 1;
}

int partida_confirmar(PartidaGui *p) {
    /* Trava: nas telas que esperam clique, o primeiro 0,3 s nao conta. */
    if (p->tempoNoEstado < PARTIDA_TEMPO_TRAVA) return 0;

    switch (p->estado) {
    case PARTIDA_COBERTURA:
        mudar_estado(p, PARTIDA_VEZ_DO_JOGADOR);
        return 1;
    case PARTIDA_FIM_MAO:
        /* main.c 509-510: a partida acaba quando uma dupla chega a 12. */
        if (p->mesa.pontosDupla1 >= PONTOS_PARA_VENCER || p->mesa.pontosDupla2 >= PONTOS_PARA_VENCER) {
            p->duplaVencedoraPartida = (p->mesa.pontosDupla1 >= PONTOS_PARA_VENCER) ? 0 : 1;
            mudar_estado(p, PARTIDA_FIM_PARTIDA);
        } else {
            mudar_estado(p, PARTIDA_INICIO_MAO);
        }
        return 1;
    case PARTIDA_FIM_PARTIDA:
        partida_iniciar(p);
        return 1;
    default:
        /* VEZ_DO_JOGADOR espera uma carta, nao um clique solto;
           CARTA_VOANDO e APURANDO ignoram de proposito. */
        return 0;
    }
}

/* ------------------------------------------------------------- o tempo */

void partida_atualizar(PartidaGui *p, float dt) {
    p->tempoNoEstado += dt;

    switch (p->estado) {
    case PARTIDA_INICIO_MAO:
        comecar_mao(p);
        break;
    case PARTIDA_CARTA_VOANDO:
        if (p->tempoNoEstado < PARTIDA_TEMPO_VOO) break;
        /* main.c 369: a rodada segue ate as 4 cartas estarem na mesa. */
        if (rodada_completa(&p->mao.rodadas[p->mao.rodada_atual])) apurar(p);
        else chamar_proximo(p);
        break;
    case PARTIDA_APURANDO:
        if (p->tempoNoEstado < PARTIDA_TEMPO_APURACAO) break;
        if (mao_terminou(&p->mao)) {
            encerrar_mao(p);   /* main.c 460: a mao acabou */
        } else {
            /* main.c 434: quem venceu abre a proxima rodada; no empate,
               mao.jogador_abre nao muda (mao.c) e abre quem abriu esta. */
            if (!definirPrimeiroDaMao(&p->mesa, p->mao.jogador_abre)) falhou(p, "definirPrimeiroDaMao");
            comecar_rodada(p);
        }
        break;
    default:
        /* Os outros estados so mudam por clique ou carta. */
        break;
    }
}

/* ------------------------------------------------------------ consultas */

int partida_assento_visto(const PartidaGui *p) {
    /* assentoDaVez so muda quando o proximo e chamado: em APURANDO e nas
       telas de fim a camera fica com o ultimo que jogou. */
    return p->assentoDaVez;
}

int partida_mao_visivel(const PartidaGui *p) {
    /* Privacidade: so quem esta na vez ve a propria mao. Na cobertura, na
       apuracao e nas telas de fim todos olham a tela, entao ninguem ve. */
    if (p->estado == PARTIDA_VEZ_DO_JOGADOR || p->estado == PARTIDA_CARTA_VOANDO) return p->assentoDaVez;
    return -1;
}
