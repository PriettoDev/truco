/*
 * teste_partida.c - teste automatico de partida_gui.c, sem janela.
 *
 * Joga 200 partidas escolhendo cartas ao acaso (semente fixa, para o
 * resultado ser sempre o mesmo) e confere que:
 *  - toda partida termina com uma dupla em 12;
 *  - nenhuma funcao de src/ falhou;
 *  - nenhuma carta se repete entre as 12 distribuidas e a vira, e as
 *    jogadas mais as que sobraram sao exatamente as distribuidas;
 *  - quem abre a mao gira 0, 1, 2, 3; o vencedor abre a rodada seguinte e,
 *    no empate, abre quem abriu; a ordem e anti-horaria;
 *  - a mao so aparece na propria vez e as travas de clique funcionam;
 *  - o destaque da apuracao segue a regra do rodada_apurar.
 * Tambem roda casos montados de empate e de parceiros com cartas iguais.
 *
 * Rodar: make -C gui teste (sai com codigo 1 se algo falhar).
 */
#include <stdio.h>
#include <stdlib.h>
#include "partida_gui.h"

static int erros = 0;
#define CONFERE(cond, ...) do { if (!(cond)) { erros++; if (erros <= 20) { printf("ERRO: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

static int mesma(Carta a, Carta b) { return a.naipe == b.naipe && a.valor == b.valor; }

/* Estatisticas */
static int totMaos, totRodadas, totEmpates, totParceirosIguais, totMaosEmpatadas, totTravas;

/* Avanca ate sair do estado de espera, passando tempo pequeno. */
static void esperar(PartidaGui *p, float t) { partida_atualizar(p, t); }

/* Confere as regras de travamento e privacidade no estado atual. */
static void conferir_estado(PartidaGui *p) {
    EstadoPartida e = p->estado;
    int visivel = partida_mao_visivel(p);
    if (e == PARTIDA_VEZ_DO_JOGADOR || e == PARTIDA_CARTA_VOANDO)
        CONFERE(visivel == p->assentoDaVez, "mao deveria aparecer no estado %d", (int)e);
    else
        CONFERE(visivel == -1, "mao visivel no estado %d", (int)e);
    if (e == PARTIDA_CARTA_VOANDO || e == PARTIDA_APURANDO) {
        int antes = p->estado;
        CONFERE(partida_confirmar(p) == 0 && (int)p->estado == antes, "clique aceito no estado %d", antes);
        totTravas++;
    }
}

/* Confere o destaque logo que a apuracao comeca. */
static void conferir_apuracao(PartidaGui *p) {
    const Rodada *r = &p->mao.rodadas[p->mao.rodada_atual - 1];
    Valor m = p->mao.manilha;
    totRodadas++;
    CONFERE(p->resultadoRodada == r->dupla_vencedora, "resultado diferente do modulo");
    /* parceiros com cartas iguais (mesma forca) nesta rodada */
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 4; j++)
            if (r->jogadas[i].dupla == r->jogadas[j].dupla &&
                carta_comparar(r->jogadas[i].carta, r->jogadas[j].carta, m) == 0) totParceirosIguais++;

    if (r->dupla_vencedora == EMPATE) {
        totEmpates++;
        CONFERE(p->destaque[0] >= 0 && p->destaque[1] >= 0, "empate sem dois destaques");
        if (p->destaque[0] >= 0 && p->destaque[1] >= 0) {
            CartaNaMesa a = p->naMesa[p->destaque[0]], b = p->naMesa[p->destaque[1]];
            CONFERE(a.assento % 2 == 0 && b.assento % 2 == 1, "destaques do empate nao sao um de cada dupla");
            CONFERE(carta_comparar(a.carta, b.carta, m) == 0, "destaques do empate com forcas diferentes");
            for (int i = 0; i < p->qtdNaMesa; i++)
                CONFERE(carta_forca(p->naMesa[i].carta, m) <= carta_forca(a.carta, m), "carta mais forte que o destaque do empate");
        }
    } else {
        CONFERE(p->destaque[0] >= 0 && p->destaque[1] == -1, "vitoria com destaque errado");
        if (p->destaque[0] >= 0) {
            CartaNaMesa v = p->naMesa[p->destaque[0]];
            CONFERE(v.assento == r->jogador_vencedor, "destaque nao e do jogador vencedor");
            CONFERE(v.assento % 2 == r->dupla_vencedora, "destaque de outra dupla");
            for (int i = 0; i < p->qtdNaMesa; i++)
                if (p->naMesa[i].assento % 2 != v.assento % 2)
                    CONFERE(carta_comparar(v.carta, p->naMesa[i].carta, m) > 0, "vencedora nao vence o adversario");
        }
    }
}

/* Uma partida inteira com cartas ao acaso. Devolve 1 se terminou. */
static int jogar_partida(PartidaGui *p, int numero) {
    Carta dadas[13];       /* 12 distribuidas + vira */
    Carta jogadas[12];
    int qtdJogadas = 0, numMao = 0, abreEsperado = 0, abreRodada = -1;
    int passos = 0;

    partida_iniciar(p);
    while (p->estado != PARTIDA_FIM_PARTIDA) {
        if (++passos > 100000) { CONFERE(0, "partida %d nao terminou", numero); return 0; }
        conferir_estado(p);

        switch (p->estado) {
        case PARTIDA_INICIO_MAO:
            esperar(p, 0.016f);
            /* acabou de distribuir: confere cartas repetidas e quem abre */
            numMao++;
            totMaos++;
            for (int s = 0; s < 4; s++) {
                CONFERE(p->mesa.jogadores[s].qtdCartas == 3, "jogador %d sem 3 cartas", s);
                for (int c = 0; c < 3; c++) dadas[s * 3 + c] = p->mesa.jogadores[s].mao[c];
            }
            dadas[12] = p->mesa.vira;
            for (int i = 0; i < 13; i++)
                for (int j = i + 1; j < 13; j++)
                    CONFERE(!mesma(dadas[i], dadas[j]), "carta repetida na mao %d da partida %d", numMao, numero);
            CONFERE(p->mao.jogador_abre == abreEsperado, "mao %d abriu com %d, esperado %d",
                    numMao, p->mao.jogador_abre, abreEsperado);
            CONFERE(p->assentoDaVez == abreEsperado, "primeiro da vez errado");
            CONFERE(p->mao.manilha == manilha_da_vira(p->mesa.vira), "manilha errada");
            abreRodada = abreEsperado;
            abreEsperado = (abreEsperado + 1) % 4;
            qtdJogadas = 0;
            break;
        case PARTIDA_COBERTURA:
            /* clique cedo demais (clique duplo) tem que ser ignorado */
            CONFERE(partida_confirmar(p) == 0 && p->estado == PARTIDA_COBERTURA, "cobertura aceitou clique antes de 0,3 s");
            esperar(p, 0.31f);
            CONFERE(partida_confirmar(p) == 1 && p->estado == PARTIDA_VEZ_DO_JOGADOR, "cobertura nao abriu");
            break;
        case PARTIDA_VEZ_DO_JOGADOR: {
            Jogador *j = &p->mesa.jogadores[p->assentoDaVez];
            int qtd = j->qtdCartas;
            int pos = rand() % qtd;
            Carta escolhida = j->mao[pos];
            if (p->qtdNaMesa == 0)
                CONFERE(p->assentoDaVez == abreRodada, "rodada aberta por %d, esperado %d", p->assentoDaVez, abreRodada);
            else
                CONFERE(p->assentoDaVez == (p->naMesa[p->qtdNaMesa - 1].assento + 1) % 4, "ordem anti-horaria quebrada");
            CONFERE(partida_jogar_carta(p, pos) == 0, "carta saiu antes da trava");
            esperar(p, 0.31f);
            CONFERE(partida_jogar_carta(p, qtd) == 0, "posicao invalida aceita");
            CONFERE(partida_jogar_carta(p, pos) == 1, "carta valida recusada");
            CONFERE(j->qtdCartas == qtd - 1, "carta nao saiu da mao");
            CONFERE(mesma(p->naMesa[p->qtdNaMesa - 1].carta, escolhida), "carta na mesa diferente da escolhida");
            jogadas[qtdJogadas++] = escolhida;
            break;
        }
        case PARTIDA_CARTA_VOANDO:
            esperar(p, 0.2f);
            CONFERE(p->estado == PARTIDA_CARTA_VOANDO, "voo acabou antes de 0,35 s");
            esperar(p, 0.2f);
            if (p->estado == PARTIDA_APURANDO) conferir_apuracao(p);
            break;
        case PARTIDA_APURANDO: {
            int resultado = p->resultadoRodada, abreAntes = abreRodada;
            int vencedor = p->mao.rodadas[p->mao.rodada_atual - 1].jogador_vencedor;
            esperar(p, 1.0f);
            CONFERE(p->estado == PARTIDA_APURANDO, "apuracao acabou antes de 1,5 s");
            esperar(p, 0.6f);
            if (p->estado == PARTIDA_COBERTURA) {
                /* vencedor abre; no empate abre quem abriu */
                abreRodada = (resultado == EMPATE) ? abreAntes : vencedor;
                CONFERE(p->qtdNaMesa == 0, "mesa nao foi limpa");
            }
            break;
        }
        case PARTIDA_FIM_MAO: {
            /* jogadas + as que sobraram na mao = distribuidas */
            int total = qtdJogadas;
            Carta todas[12];
            for (int i = 0; i < qtdJogadas; i++) todas[i] = jogadas[i];
            for (int s = 0; s < 4; s++)
                for (int c = 0; c < p->mesa.jogadores[s].qtdCartas; c++) todas[total++] = p->mesa.jogadores[s].mao[c];
            CONFERE(total == 12, "faltam cartas no fim da mao (%d)", total);
            for (int i = 0; i < 12 && total == 12; i++) {
                int achou = 0;
                for (int k = 0; k < 12; k++) achou += mesma(dadas[i], todas[k]);
                CONFERE(achou == 1, "carta distribuida aparece %d vez(es) no fim da mao", achou);
            }
            if (p->duplaVencedoraMao == MAO_EMPATE) {
                totMaosEmpatadas++;
                CONFERE(p->pontosDaMao == 0, "mao empatada pontuou");
            } else {
                CONFERE(p->pontosDaMao == 1, "mao sem truco valeu %d", p->pontosDaMao);
            }
            CONFERE(p->mesa.numeroDaRodada == numMao + 1, "numero da mao nao avancou");
            CONFERE(partida_confirmar(p) == 0, "fim da mao aceitou clique antes de 0,3 s");
            esperar(p, 0.31f);
            CONFERE(partida_confirmar(p) == 1, "fim da mao nao avancou");
            break;
        }
        default:
            esperar(p, 0.016f);
            break;
        }
    }
    {
        int a = partida_pontos(p, 0), b = partida_pontos(p, 1), v = p->duplaVencedoraPartida;
        CONFERE((a == 12) != (b == 12), "placar final %d x %d", a, b);
        CONFERE(v >= 0 && partida_pontos(p, v) == 12, "vencedora %d com placar %d x %d", v, a, b);
    }
    return 1;
}

/* ---- casos montados: troca as cartas logo depois da distribuicao ---- */

/* Joga a rodada inteira com a 1a carta de cada um e devolve o estado da apuracao. */
static void jogar_rodada_montada(PartidaGui *p, Carta vira, const Carta porAssento[4], const char *nome,
                                 int resultadoEsperado, int dest0, int dest1) {
    partida_iniciar(p);
    partida_atualizar(p, 0.016f);              /* INICIO_MAO -> COBERTURA, assento 0 abre */
    p->mesa.vira = vira;
    p->manilha = manilha_da_vira(vira);
    p->mao.manilha = p->manilha;
    for (int s = 0; s < 4; s++) p->mesa.jogadores[s].mao[0] = porAssento[s];

    for (int k = 0; k < 4; k++) {
        partida_atualizar(p, 0.31f);
        partida_confirmar(p);
        partida_atualizar(p, 0.31f);
        partida_jogar_carta(p, 0);
        partida_atualizar(p, 0.4f);
    }
    CONFERE(p->estado == PARTIDA_APURANDO, "%s: nao chegou na apuracao", nome);
    CONFERE(p->resultadoRodada == resultadoEsperado, "%s: resultado %d, esperado %d", nome, p->resultadoRodada, resultadoEsperado);
    {
        int a0 = p->destaque[0] >= 0 ? p->naMesa[p->destaque[0]].assento : -1;
        int a1 = p->destaque[1] >= 0 ? p->naMesa[p->destaque[1]].assento : -1;
        CONFERE(a0 == dest0 && a1 == dest1, "%s: destaques nos assentos %d e %d, esperado %d e %d",
                nome, a0, a1, dest0, dest1);
        printf("  %-46s resultado %2d | destaque assentos %2d e %2d\n", nome, p->resultadoRodada, a0, a1);
    }
    CONFERE(p->falhas == 0, "%s: %d falhas", nome, p->falhas);
}

int main(void) {
    static PartidaGui p;   /* static: nunca copiar, e a struct e grande */
    int falhasTotais = 0, terminaram = 0, vitorias[2] = { 0, 0 };
    Carta vira = { OUROS, QUATRO };   /* manilha = CINCO */

    srand(12345);
    for (int i = 0; i < 200; i++) {
        terminaram += jogar_partida(&p, i);
        falhasTotais += p.falhas;
        if (p.duplaVencedoraPartida == 0 || p.duplaVencedoraPartida == 1) vitorias[p.duplaVencedoraPartida]++;
    }

    printf("Casos montados (vira 4 de ouros, manilha 5):\n");
    {   /* empate entre duplas: 3 x 3, os parceiros com 4 */
        Carta c[4] = { { ESPADAS, TRES }, { COPAS, TRES }, { PAUS, QUATRO }, { COPAS, QUATRO } };
        jogar_rodada_montada(&p, vira, c, "empate entre duplas (3 x 3)", EMPATE, 0, 1);
    }
    {   /* parceiros com cartas iguais: 3 e 3 contra 2 e 2 -> dupla 0 vence, fica o 1o 3 */
        Carta c[4] = { { ESPADAS, TRES }, { COPAS, DOIS }, { PAUS, TRES }, { OUROS, DOIS } };
        jogar_rodada_montada(&p, vira, c, "parceiros iguais (3 e 3 contra 2 e 2)", 0, 0, -1);
    }
    {   /* parceiros iguais dos dois lados e empate: destaca o 1o de cada dupla */
        Carta c[4] = { { ESPADAS, REI }, { COPAS, REI }, { PAUS, REI }, { OUROS, REI } };
        jogar_rodada_montada(&p, vira, c, "quatro reis (empate, 1o de cada dupla)", EMPATE, 0, 1);
    }
    {   /* manilha desempata pelo naipe: 5 de paus vence 5 de copas */
        Carta c[4] = { { COPAS, CINCO }, { PAUS, CINCO }, { ESPADAS, TRES }, { OUROS, TRES } };
        jogar_rodada_montada(&p, vira, c, "manilhas (5 de paus vence 5 de copas)", 1, 1, -1);
    }

    printf("\nRESUMO\n");
    printf("  partidas jogadas ate o fim : %d de 200\n", terminaram);
    printf("  vitorias por dupla interna : dupla 0 (Seu Bigode/Tonhão) %d | dupla 1 (Dona Marlene/Kiko) %d\n",
           vitorias[0], vitorias[1]);
    printf("  maos / rodadas             : %d / %d\n", totMaos, totRodadas);
    printf("  rodadas empatadas          : %d\n", totEmpates);
    printf("  parceiros com cartas iguais: %d\n", totParceirosIguais);
    printf("  maos empatadas (0 pontos)  : %d\n", totMaosEmpatadas);
    printf("  cliques bloqueados conferidos: %d\n", totTravas);
    printf("  falhas de funcoes de src/  : %d\n", falhasTotais);
    printf("  erros do teste             : %d\n", erros);
    CONFERE(totEmpates > 0 && totParceirosIguais > 0, "faltou caso natural de empate ou parceiros iguais");
    printf("%s\n", (erros == 0 && falhasTotais == 0 && terminaram == 200) ? "TUDO CERTO" : "FALHOU");
    return erros != 0 || falhasTotais != 0;
}
