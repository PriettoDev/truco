#ifndef ESTATISTICAS_H
#define ESTATISTICAS_H

#include "jogador.h"
#include "historico.h"

void estatisticas_exibir_mao_mais_cara(const tp_listad *historico);
void estatisticas_exibir_taxa_correu(const tp_listad *historico, const Jogador jogadores[4]);

#endif
