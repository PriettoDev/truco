#include <stdio.h>
#include "../src/mao.h"

static int falhas = 0;

/* Imprime OK ou ERRO e conta as falhas */
static void confere(const char *descricao, int obtido, int esperado) {
    if (obtido == esperado) {
        printf("OK    %s\n", descricao);
    } else {
        printf("ERRO  %s (obtido %d, esperado %d)\n", descricao, obtido, esperado);
        falhas++;
    }
}

/* Faz os assentos 0, 1, 2 e 3 jogarem uma carta cada (a dupla do assento e assento % 2)
   e apura a rodada. Devolve o retorno de mao_apurar_rodada. */
static int jogaRodada(Mao *m, Valor v0, Valor v1, Valor v2, Valor v3) {
    Carta c0 = {OUROS, v0};
    Carta c1 = {OUROS, v1};
    Carta c2 = {OUROS, v2};
    Carta c3 = {OUROS, v3};
    mao_jogar(m, c0, 0, 0);
    mao_jogar(m, c1, 1, 1);
    mao_jogar(m, c2, 2, 0);
    mao_jogar(m, c3, 3, 1);
    return mao_apurar_rodada(m);
}

/* Rodada em que a dupla 0 vence (assento 0 joga o TRES) */
static int duplaZeroVence(Mao *m) { return jogaRodada(m, TRES, QUATRO, QUATRO, QUATRO); }
/* Rodada em que a dupla 1 vence (assento 3 joga o TRES) */
static int duplaUmVence(Mao *m)   { return jogaRodada(m, QUATRO, QUATRO, QUATRO, TRES); }
/* Rodada empatada (todos jogam QUATRO, nenhuma e manilha) */
static int empata(Mao *m)         { return jogaRodada(m, QUATRO, QUATRO, QUATRO, QUATRO); }

int main(void) {
    Mao m;
    Valor manilha = SETE;   /* ninguem joga SETE nestes testes */

    printf("--- aposta ---\n");
    mao_iniciar(&m, manilha, 0);
    confere("mao comeca valendo 1", m.aposta.valor, 1);
    confere("dupla 0 pede truco", mao_pedir(&m, 0), 1);
    confere("dupla 0 nao responde o proprio pedido", mao_aceitar(&m, 0), 0);
    confere("nao pode jogar carta com pedido pendente", mao_jogar(&m, (Carta){OUROS, TRES}, 0, 0), 0);
    confere("dupla 1 aceita", mao_aceitar(&m, 1), 1);
    confere("mao passa a valer 3", m.aposta.valor, 3);
    confere("dupla 0 nao pede duas vezes seguidas", mao_pedir(&m, 0), 0);
    confere("dupla 1 pede seis", mao_pedir(&m, 1), 1);
    confere("dupla 0 aceita o seis", mao_aceitar(&m, 0), 1);
    confere("mao passa a valer 6", m.aposta.valor, 6);

    printf("--- correr ---\n");
    mao_iniciar(&m, manilha, 0);
    mao_pedir(&m, 0);
    confere("correr do truco: mao encerrada", mao_correr(&m, 1), 1);
    confere("quem pediu (dupla 0) vence", m.dupla_vencedora, 0);
    confere("correr do truco vale 1 ponto", m.pontos_ganhos, 1);

    mao_iniciar(&m, manilha, 0);
    mao_pedir(&m, 0);
    mao_pedir(&m, 1);
    mao_pedir(&m, 0);
    confere("truco, seis, doze: dupla 1 corre", mao_correr(&m, 1), 1);
    confere("dupla 0 vence", m.dupla_vencedora, 0);
    confere("correr do doze vale 6", m.pontos_ganhos, 6);

    printf("--- resultado da mao ---\n");
    mao_iniciar(&m, manilha, 0);
    confere("apurar antes da rodada completa", mao_apurar_rodada(&m), 0);
    duplaZeroVence(&m);
    confere("1 vitoria: mao ainda nao terminou", mao_terminou(&m), 0);
    duplaZeroVence(&m);
    confere("2 vitorias: mao terminou", mao_terminou(&m), 1);
    confere("dupla 0 venceu a mao", m.dupla_vencedora, 0);
    confere("vale 1 ponto", m.pontos_ganhos, 1);

    mao_iniciar(&m, manilha, 0);
    empata(&m);
    duplaUmVence(&m);
    confere("empate na 1a, dupla 1 vence a 2a: mao termina", mao_terminou(&m), 1);
    confere("dupla 1 leva a mao", m.dupla_vencedora, 1);

    mao_iniciar(&m, manilha, 0);
    duplaZeroVence(&m);
    empata(&m);
    confere("vitoria na 1a e empate na 2a: mao termina", mao_terminou(&m), 1);
    confere("prevalece a 1a (dupla 0)", m.dupla_vencedora, 0);

    mao_iniciar(&m, manilha, 0);
    duplaZeroVence(&m);
    duplaUmVence(&m);
    confere("1 a 1: mao continua", mao_terminou(&m), 0);
    empata(&m);
    confere("3a empata: vence quem ganhou a 1a (dupla 0)", m.dupla_vencedora, 0);

    mao_iniciar(&m, manilha, 0);
    empata(&m);
    empata(&m);
    empata(&m);
    confere("tres empates: mao empatada", m.dupla_vencedora, MAO_EMPATE);
    confere("mao empatada vale 0 ponto", m.pontos_ganhos, 0);

    printf("--- quem abre a proxima rodada ---\n");
    mao_iniciar(&m, manilha, 0);
    duplaUmVence(&m);
    confere("assento 3 venceu: ele abre a proxima", m.jogador_abre, 3);
    empata(&m);
    confere("em empate, quem abriu continua abrindo", m.jogador_abre, 3);

    printf("\n%s (%d falha(s))\n", falhas == 0 ? "TUDO CERTO" : "HA FALHAS", falhas);
    return falhas != 0;
}