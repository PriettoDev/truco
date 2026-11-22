#include "mao.h"

//inicia as rodadas e a aposta de uma nova mao
void mao_iniciar(Mao *m, Valor manilha, int jogador_abre) {
    int i;
    if (m == 0) return;
    m->manilha = manilha;
    m->rodada_atual = 0;
    m->vitorias[0] = 0;
    m->vitorias[1] = 0;
    m->jogador_abre = jogador_abre;
    m->dupla_vencedora = MAO_SEM_VENCEDOR;
    m->encerrada = 0;
    m->pontos_ganhos = 0;
    aposta_iniciar(&m->aposta);
    for (i = 0; i < MAX_RODADAS_MAO; i++) {
        rodada_iniciar(&m->rodadas[i]);
        m->resultados[i] = INDEFINIDO;
    }
}

//registra uma carta na rodada atual usando o modulo do breno
int mao_jogar(Mao *m, Carta carta, int jogador, int dupla) {
    if (m == 0 || m->encerrada || aposta_pendente(&m->aposta) ||
        m->rodada_atual >= MAX_RODADAS_MAO) return 0;
    return rodada_jogar(&m->rodadas[m->rodada_atual], carta, jogador, dupla);
}

//encerra a mao com o resultado e o valor confirmado da aposta
static void finalizar_mao(Mao *m, int dupla) {
    m->dupla_vencedora = dupla;
    m->pontos_ganhos = (dupla == MAO_EMPATE) ? 0 : m->aposta.valor;
    m->encerrada = 1;
}

//apura uma rodada completa e verifica as regras de vitoria
int mao_apurar_rodada(Mao *m) {
    int n, vencedor, anterior;
    Rodada *r;
    if (m == 0 || m->encerrada || aposta_pendente(&m->aposta) ||
        m->rodada_atual >= MAX_RODADAS_MAO) return 0;
    n = m->rodada_atual;
    r = &m->rodadas[n];
    if (!rodada_completa(r) || m->resultados[n] != INDEFINIDO) return 0;
    rodada_apurar(r, m->manilha);
    vencedor = r->dupla_vencedora;
    if (vencedor == INDEFINIDO) return 0;
    m->resultados[n] = vencedor;
    if (vencedor == 0 || vencedor == 1) {
        m->vitorias[vencedor]++;
        m->jogador_abre = r->jogador_vencedor;
    }
    //em empate, o jogador que abriu a rodada continua abrindo

    if (m->vitorias[0] == 2) finalizar_mao(m, 0);
    else if (m->vitorias[1] == 2) finalizar_mao(m, 1);
    else if (n == 1 && m->resultados[0] == EMPATE && vencedor != EMPATE) {
        //empate na primeira: quem vence a segunda leva a mao
        finalizar_mao(m, vencedor);
    } else if (n == 1 && vencedor == EMPATE && m->resultados[0] != EMPATE) {
        //vitoria na primeira e empate na segunda: prevalece a primeira
        finalizar_mao(m, m->resultados[0]);
    } else if (n == 2) {
        //na terceira, empate favorece quem ganhou a primeira rodada
        anterior = m->resultados[0];
        if (vencedor != EMPATE) finalizar_mao(m, vencedor);
        else if (anterior != EMPATE) finalizar_mao(m, anterior);
        else finalizar_mao(m, MAO_EMPATE);
    }
    m->rodada_atual++;
    return 1;
}

//encaminha o pedido ao modulo de aposta
int mao_pedir(Mao *m, int dupla) {
    if (m == 0 || m->encerrada) return 0;
    return aposta_pedir(&m->aposta, dupla);
}

//confirma um pedido pendente
int mao_aceitar(Mao *m, int dupla) {
    if (m == 0 || m->encerrada) return 0;
    return aposta_aceitar(&m->aposta, dupla);
}

//encerra a mao antecipadamente quando uma dupla corre
int mao_correr(Mao *m, int dupla) {
    if (m == 0 || m->encerrada || !aposta_correr(&m->aposta, dupla)) return 0;
    m->dupla_vencedora = m->aposta.dupla_vencedora;
    m->pontos_ganhos = m->aposta.pontos_ganhos;
    m->encerrada = 1;
    return 1;
}

//informa se a mao ja terminou
int mao_terminou(const Mao *m) {
    return m != 0 && m->encerrada;
}
