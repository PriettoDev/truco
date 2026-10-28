#include "rodada.h"              

void rodada_iniciar(Rodada *r) {
    r->qtd = 0;                          /* mesa vazia: proxima carta vai em jogadas[0] */
    r->dupla_vencedora  = INDEFINIDO;    /* ainda nao verificada */
    r->jogador_vencedor = -1;            /* ninguem fez a rodada */
    /* o vetor jogadas nao precisa ser limpo: so as posicoes abaixo de qtd sao lidas */
}

int rodada_jogar(Rodada *r, Carta c, int jogador, int dupla) {
    if (r->qtd == TOTAL_CARTAS) {        /* mesa cheia: escrever em jogadas[4] corromperia a struct */
        return 0;
    }
    if (jogador < 0 || jogador > 3 || (dupla != 0 && dupla != 1)) {
        return 0;                        /* valida na entrada: a apuracao usa dupla como indice */
    }
    r->jogadas[r->qtd].carta   = c;      /* r-> (ponteiro), jogadas[qtd] (struct), .carta */
    r->jogadas[r->qtd].jogador = jogador;
    r->jogadas[r->qtd].dupla   = dupla;
    r->qtd++;                            /* incrementa DEPOIS de escrever */
    return 1;
}

int rodada_completa(const Rodada *r) {   /* const: so le, nao altera */
    return r->qtd == TOTAL_CARTAS;       /* a comparacao ja devolve 1 ou 0 */
}

void rodada_apurar(Rodada *r, Valor manilha) {
    int melhor[2] = { -1, -1 };          /* posicao da candidata de cada dupla; -1 = sem candidata */
    int i, d, resultado;

    if (!rodada_completa(r)) {           /* mesa incompleta: INDEFINIDO */
        return;
    }

    /* Fase 1: melhor carta de cada dupla */
    for (i = 0; i < r->qtd; i++) {       /* i = posicao da carta na mesa, nao o numero da rodada */
        d = r->jogadas[i].dupla;         /* de que dupla e esta carta: vira indice de melhor[] */
        if (melhor[d] == -1 ||           /* dupla sem candidata: entra direto (e protege a linha abaixo) */
            carta_comparar(r->jogadas[i].carta,           /* carta atual */
                           r->jogadas[melhor[d]].carta,   /* candidata atual da dupla */
                           manilha) > 0) {                /* > 0 estrito: em empate fica quem jogou primeiro */
            melhor[d] = i;               /* guarda a posicao, para depois ter carta E jogador */
        }
    }
    if (melhor[0] == -1 || melhor[1] == -1) {   /* uma dupla sem carta: erro de quem montou a mesa */
        return;
    }

    /* Fase 2: comparação entre cartas das duplas */
    resultado = carta_comparar(r->jogadas[melhor[0]].carta,
                               r->jogadas[melhor[1]].carta, manilha);

    if (resultado > 0) {                 /* dupla 0 ganhou */
        r->dupla_vencedora  = 0;
        r->jogador_vencedor = r->jogadas[melhor[0]].jogador;
    } else if (resultado < 0) {          /* dupla 1 ganhou */
        r->dupla_vencedora  = 1;
        r->jogador_vencedor = r->jogadas[melhor[1]].jogador;
    } else {                             /* melhores das duas duplas com mesma forca */
        r->dupla_vencedora  = EMPATE;
        r->jogador_vencedor = -1;
    }
}