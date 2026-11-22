#include <stdio.h>
#include "estatisticas.h"

// procura no historico a mao com o maior valor
void estatisticas_exibir_mao_mais_cara(const tp_listad *historico) {
    int maiorValor = 0;
    int numeroMao = 0;

    if (verificarHistoricoVazio(historico)) {
        printf("\nNenhuma mao registrada.\n");
        return;
    }

    for (const tp_no *no = obterPrimeiraRodadaVar(historico); no != NULL; no = avancarVar(no)) {
        if (no->info.valorMao > maiorValor) {
            maiorValor = no->info.valorMao;
            numeroMao = no->info.numeroMao;
        }
    }

    printf("\nMao mais cara: mao %d, valendo %d ponto(s).\n", numeroMao, maiorValor);
}

// taxa = vezes que correu / vezes que respondeu (aceitou ou correu)
void estatisticas_exibir_taxa_correu(const tp_listad *historico, const Jogador jogadores[4]) {
    int respondeu[4] = {0, 0, 0, 0};
    int correu[4] = {0, 0, 0, 0};

    if (jogadores == NULL) return;

    for (const tp_no *no = obterPrimeiraRodadaVar(historico); no != NULL; no = avancarVar(no)) {
        for (int i = 0; i < no->info.qtdEventos; i++) {
            EventoAposta e = no->info.eventos[i];
            if (e.tipo == VAR_ACEITOU || e.tipo == VAR_CORREU) {
                respondeu[e.indiceJogador]++;
                if (e.tipo == VAR_CORREU) correu[e.indiceJogador]++;
            }
        }
    }

    printf("\n===== TAXA DE CORREU DO TRUCO =====\n");
    for (int i = 0; i < 4; i++) {
        if (respondeu[i] == 0) {
            printf("%s: nao respondeu a nenhum pedido.\n", jogadores[i].nome);
        } else {
            printf("%s: correu %d de %d vez(es) = %.1f%%\n", jogadores[i].nome,
                   correu[i], respondeu[i], 100.0 * correu[i] / respondeu[i]);
        }
    }
}
