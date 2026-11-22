#ifndef MAO_H
#define MAO_H

#include "rodada.h"
#include "aposta.h"

#define MAX_RODADAS_MAO 3
#define MAO_SEM_VENCEDOR -2
#define MAO_EMPATE -1

typedef struct {
    Rodada rodadas[MAX_RODADAS_MAO];
    Aposta aposta;
    Valor manilha;
    int rodada_atual;          //indice da rodada em andamento
    int resultados[MAX_RODADAS_MAO];
    int vitorias[2];
    int jogador_abre;          //indice de 0 a 3 na mesa
    int dupla_vencedora;       //0 ou 1, ou resultado indefinido
    int encerrada;
    int pontos_ganhos;
} Mao;

void mao_iniciar(Mao *m, Valor manilha, int jogador_abre);
int mao_jogar(Mao *m, Carta carta, int jogador, int dupla);
int mao_apurar_rodada(Mao *m);
int mao_pedir(Mao *m, int dupla);
int mao_aceitar(Mao *m, int dupla);
int mao_correr(Mao *m, int dupla);
int mao_terminou(const Mao *m);

#endif
