#include <stdio.h>
#include <stdlib.h>   // CORRIGIDO: necessario para malloc e free
#include "mesa.h"

/////////////////////////////////////////////////////////////////
// LISTA DUPLAMENTE ENCADEADA (ordem dos jogadores)
/////////////////////////////////////////////////////////////////

tp_listad *inicializa_listad(void) {
    tp_listad *lista = malloc(sizeof(tp_listad));

    if (lista == NULL) {
        return NULL;
    }

    lista->ini = NULL;
    lista->fim = NULL;

    return lista;
}

int listad_vazia(tp_listad *lista) {
    return lista == NULL || lista->ini == NULL;
}

tp_no *aloca(void) {
    return malloc(sizeof(tp_no));
}

int insere_listad_no_fim(tp_listad *lista, tp_item e) {
    if (lista == NULL) {
        return 0;
    }

    tp_no *novo = aloca();

    if (novo == NULL) {
        return 0;
    }

    novo->info = e;
    novo->prox = NULL;
    novo->ant = lista->fim;

    if (listad_vazia(lista)) {
        lista->ini = novo;
    } else {
        lista->fim->prox = novo;
    }

    lista->fim = novo;

    return 1;
}

int remove_listad(tp_listad *lista, tp_item e) {
    if (lista == NULL) {
        return 0;
    }

    tp_no *atu = lista->ini;

    while (atu != NULL && atu->info != e) {
        atu = atu->prox;
    }

    if (atu == NULL) {
        return 0;
    }

    if (atu->ant != NULL) {
        atu->ant->prox = atu->prox;
    } else {
        lista->ini = atu->prox;
    }

    if (atu->prox != NULL) {
        atu->prox->ant = atu->ant;
    } else {
        lista->fim = atu->ant;
    }

    free(atu);

    return 1;
}

void imprime_listad(tp_listad *lista, int ordem) {
    if (lista == NULL) {
        printf("Lista nao inicializada.\n");
        return;
    }

    tp_no *atu;

    if (ordem == 1) {
        atu = lista->ini;

        while (atu != NULL) {
            printf("%d ", atu->info);
            atu = atu->prox;
        }
    } else if (ordem == 2) {
        atu = lista->fim;

        while (atu != NULL) {
            printf("%d ", atu->info);
            atu = atu->ant;
        }
    } else {
        printf("Codigo invalido");
    }

    printf("\n");
}

tp_no *busca_listade(tp_listad *lista, tp_item e) {
    if (lista == NULL) {
        return NULL;
    }

    tp_no *atu = lista->ini;

    while (atu != NULL && atu->info != e) {
        atu = atu->prox;
    }

    return atu;
}

tp_listad *Destroi_listad(tp_listad *lista) {
    if (lista == NULL) {
        return NULL;
    }

    tp_no *atu = lista->ini;

    while (atu != NULL) {
        tp_no *proximo = atu->prox;
        free(atu);
        atu = proximo;
    }

    free(lista);

    return NULL;   // CORRIGIDO: faltava o return e a chave que fecha a funcao
}

////////////////////////////////////////////////////////////////////////////////
// MESA
////////////////////////////////////////////////////////////////////////////////

// Inicializa a mesa e a lista que controla a ordem dos jogadores
int inicializarMesa(Mesa *mesa) {
    if (mesa == NULL) {
        return 0;
    }

    mesa->ordem = inicializa_listad();

    if (mesa->ordem == NULL) {
        return 0;
    }

    mesa->numeroDaRodada = 1;
    mesa->inicioDaRodada = -1;
    mesa->pontosDupla1 = 0;
    mesa->pontosDupla2 = 0;

    return 1;
}

// Insere o indice do jogador no final da lista
int inserirJogadorNaOrdem(Mesa *mesa, int indice) {
    if (mesa == NULL || mesa->ordem == NULL) {
        return 0;
    }

    if (indice < 0 || indice >= MAX_JOGADOR) {
        return 0;
    }

    return insere_listad_no_fim(mesa->ordem, indice);
}

// Passa o primeiro jogador para o final da lista
void girarMesa(Mesa *mesa) {
    if (mesa == NULL || mesa->ordem == NULL) {
        return;
    }

    tp_listad *lista = mesa->ordem;

    if (listad_vazia(lista) || lista->ini == lista->fim) {
        return;
    }

    tp_no *primeiro = lista->ini;

    // o segundo jogador passa a ser o primeiro
    lista->ini = primeiro->prox;
    lista->ini->ant = NULL;

    // o antigo primeiro jogador vai para o final
    primeiro->prox = NULL;
    primeiro->ant = lista->fim;

    lista->fim->prox = primeiro;
    lista->fim = primeiro;
}

// NOVA: gira a lista ate o jogador "indice" ficar na frente (ini).
// Retorna 1 se deu certo, 0 se o jogador nao esta na lista.
int definirPrimeiroDaMao(Mesa *mesa, int indice) {
    if (mesa == NULL || mesa->ordem == NULL) {
        return 0;
    }

    if (busca_listade(mesa->ordem, indice) == NULL) {
        return 0;
    }

    while (mesa->ordem->ini->info != indice) {
        girarMesa(mesa);
    }

    return 1;
}

// A cada nova rodada, abre o proximo jogador da roda em relacao a quem abriu a anterior.
// Na primeira rodada, abre quem estiver na frente da lista.
// Retorna o indice do jogador que abre, ou -1 em caso de erro.
int iniciarNovaRodada(Mesa *mesa) {
    if (mesa == NULL || mesa->ordem == NULL || listad_vazia(mesa->ordem)) {
        return -1;
    }

    if (mesa->inicioDaRodada == -1) {
        mesa->inicioDaRodada = mesa->ordem->ini->info;
        return mesa->inicioDaRodada;
    }

    tp_no *anterior = busca_listade(mesa->ordem, mesa->inicioDaRodada);

    if (anterior == NULL) {
        return -1;
    }

    // o proximo da roda; se chegou no fim da lista, volta para o inicio
    tp_no *proximo = (anterior->prox != NULL) ? anterior->prox : mesa->ordem->ini;

    mesa->inicioDaRodada = proximo->info;
    definirPrimeiroDaMao(mesa, mesa->inicioDaRodada);

    return mesa->inicioDaRodada;
}

// NOVA: decide quem comeca a proxima mao.
// indiceVencedor = indice do jogador que venceu, ou EMPATE_MAO se empatou.
void prepararProximaMao(Mesa *mesa, int indiceVencedor) {
    if (indiceVencedor == EMPATE_MAO || !definirPrimeiroDaMao(mesa, indiceVencedor)) {
        girarMesa(mesa);   // empate: roda um passo para o lado
    }
}

// Mostra a ordem dos jogadores sem alterar a lista
void imprimirOrdemDaMao(Mesa *mesa) {
    if (mesa == NULL || mesa->ordem == NULL) {
        printf("Mesa nao inicializada.\n");
        return;
    }

    tp_no *atual = mesa->ordem->ini;

    printf("\nOrdem de jogo desta mao:\n");

    while (atual != NULL) {
        int indice = atual->info;

        if (indice >= 0 && indice < MAX_JOGADOR) {
            printf(
                "Jogador: %s (Dupla %d)\n",
                mesa->jogadores[indice].nome,
                mesa->jogadores[indice].dupla
            );
        }

        atual = atual->prox;
    }
}

// Libera a memoria utilizada pela lista
void destruirMesa(Mesa *mesa) {
    if (mesa != NULL && mesa->ordem != NULL) {
        mesa->ordem = Destroi_listad(mesa->ordem);
    }
}
