#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/pontuacao.h"

static Mesa mesa_zerada(void){
    Mesa m;
    memset(&m, 0, sizeof m);
    return m;
}


static Mao mao_apurada(int dupla, int pontos){
    Mao m;
    memset(&m, 0, sizeof m);
    m.dupla_vencedora = dupla;
    m.pontos_ganhos = pontos;
    return m;
}

static void teste_soma_normal(void) {
    Mesa m = mesa_zerada();
    Mao a = mao_apurada(0, 3);
    Mao b = mao_apurada(1, 1);
    assert(pontuacao_somar_mao(&m, &a) == 3);
    assert(m.pontosDupla1 == 3 && m.pontosDupla2 == 0);
    assert(pontuacao_somar_mao(&m, &b) == 1);
    assert(m.pontosDupla1 == 3 && m.pontosDupla2 == 1);
}

static void teste_empate_e_sem_vencedor_nao_pontuam(void) {
    Mesa m = mesa_zerada();
    Mao e = mao_apurada(MAO_EMPATE, 1);
    Mao s = mao_apurada(MAO_SEM_VENCEDOR, 1);
    assert(pontuacao_somar_mao(&m, &e) == 0);
    assert(pontuacao_somar_mao(&m, &s) == 0);
    assert(m.pontosDupla1 == 0 && m.pontosDupla2 == 0);
}

static void teste_teto_de_12(void) {
    Mesa m = mesa_zerada();
    Mao a = mao_apurada(1, 6);
    m.pontosDupla2 = 10;
    /* 10 + 6 = 16, mas o placar para em 12: entraram so 2 pontos. */
    assert(pontuacao_somar_mao(&m, &a) == 2);
    assert(m.pontosDupla2 == PONTOS_PARA_VENCER);
}

static void teste_vencedora_e_fim(void) {
    Mesa m = mesa_zerada();
    assert(pontuacao_dupla_vencedora(&m) == 0);
    assert(!pontuacao_partida_terminou(&m));

    m.pontosDupla1 = 11; m.pontosDupla2 = 11;           /* limite: ninguem fechou */
    assert(pontuacao_dupla_vencedora(&m) == 0);
    assert(!pontuacao_partida_terminou(&m));

    m.pontosDupla1 = 12;
    assert(pontuacao_dupla_vencedora(&m) == 1);          /* devolve 1, nao 0 */
    assert(pontuacao_partida_terminou(&m));

    m.pontosDupla1 = 5; m.pontosDupla2 = 12;
    assert(pontuacao_dupla_vencedora(&m) == 2);          /* devolve 2, nao 1 */
}

static void teste_ponteiros_nulos(void) {
    Mesa m = mesa_zerada();
    Mao a = mao_apurada(0, 3);
    assert(pontuacao_somar_mao(NULL, &a) == 0);
    assert(pontuacao_somar_mao(&m, NULL) == 0);
    assert(pontuacao_dupla_vencedora(NULL) == 0);
    assert(!pontuacao_partida_terminou(NULL));
}

/* Simula uma partida inteira: maos de 3, 1, 6, 3... ate alguem fechar. */
static void teste_partida_completa(void) {
    Mesa m = mesa_zerada();
    int valores[] = {3, 1, 6, 3, 1, 12};
    int duplas[]  = {0, 1, 0, 1, 0, 1};
    int i = 0;
    while (!pontuacao_partida_terminou(&m) && i < 6) {
        Mao mao = mao_apurada(duplas[i], valores[i]);
        pontuacao_somar_mao(&m, &mao);
        i++;
    }
    /* Dupla 1 (indice 0): 3+6+1 = 10. Dupla 2 (indice 1): 1+3+12 -> teto 12. */
    assert(pontuacao_dupla_vencedora(&m) == 2);
    assert(m.pontosDupla2 == 12 && m.pontosDupla1 == 10);
}

int main(void) {
    teste_soma_normal();
    teste_empate_e_sem_vencedor_nao_pontuam();
    teste_teto_de_12();
    teste_vencedora_e_fim();
    teste_ponteiros_nulos();
    teste_partida_completa();
    puts("testePontuacao: todos os testes passaram.");
    return 0;
}
