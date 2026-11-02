#include "historico.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Histórico VAR - Lista Duplamente Encadeada: cada NO armazena uma cópia de uma rodada.

// Adaptacao direta da inicializa_listad
tp_listad *inicializarHistorico(void) {
    tp_listad *lista = malloc(sizeof(*lista));
    if (lista == NULL)
        return NULL;

    lista->ini = NULL;
    lista->fim = NULL;
    lista->tamanho = 0;
    return lista;
}

int verificarHistoricoVazio(const tp_listad *lista) {
    return lista == NULL || lista->ini == NULL;
}

static tp_no *alocarNoHistorico(void) {
    return malloc(sizeof(tp_no));
}

//Preserva ondem cronológica das rodadas
int adicionarRodadaHistorico(tp_listad *lista, tp_item e) {
    if (lista == NULL || e.qtdJogadas < 0 || e.qtdJogadas > VAR_MAX_JOGADAS || e.qtdEventos < 0 || e.qtdEventos > VAR_MAX_EVENTOS) {
        return 0;
    }

    tp_no *novo = alocarNoHistorico();
    if (novo == NULL) return 0;

    novo->info = e;
    novo->ant = lista->fim;
    novo->prox = NULL;

    if (verificarHistoricoVazio(lista))
        lista->ini = novo;
    else
        lista->fim->prox = novo;

    lista->fim = novo;
    lista->tamanho++;
    return 1;
}

int finalizarPartidaHistorico(tp_listad *lista) {
    if (lista == NULL) return 0;

    lista->partidaEncerrada = 1;
    return 1;
    }

//Procura uma rodada usando o número da mão e da rodada
const tp_no *buscarRodadaHistorico(const tp_listad *lista, int mao, int rodada) {
    if (lista == NULL) return NULL;

    const tp_no *atual = lista->ini;
    while (atual != NULL) {
        if (atual->info.numeroMao == mao && atual->info.numeroRodada == rodada) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

//Liberação dos NOS e depois o descritor
tp_listad *destruirHistorico(tp_listad *lista) {
    if (lista == NULL) return NULL;

    tp_no *atual = lista->ini;
    while (atual != NULL) {
        tp_no *proximo = atual->prox;
        free(atual);
        atual = proximo;
    }
    free(lista);
    return NULL;
}

// Preenchimento de um registro de rodada
RegistroRodada criarRegistroRodada(int mao, int rodada, Carta vira, int valorConfirmado) {
    RegistroRodada registro;
    memset(&registro, 0, sizeof(registro));

    registro.numeroMao = mao;
    registro.numeroRodada = rodada;
    registro.vira = vira;
    registro.manilha_da_vira = manilha_da_vira(vira);
    registro.valorMao = valorConfirmado;
    registro.duplaVencedora = -1; // -1 = nao definida; 0 = empate

    return registro;
}

int registrarJogada(RegistroRodada *r, int indiceJogador, Carta carta) {
    if (r == NULL || r->encerradaPorCorrida || indiceJogador < 0 || indiceJogador >= VAR_MAX_JOGADAS || r->qtdJogadas < 0 || r->qtdJogadas >= VAR_MAX_JOGADAS) {
        return 0;
    }

    for (int i = 0; i < r->qtdJogadas; i++) {
        if (r->indicesJogadores[i] == indiceJogador) {
            return 0;
        }
    }
    int posicao = r->qtdJogadas;
    r->indicesJogadores[posicao] = indiceJogador;
    r->cartas[posicao] = carta;
    r->qtdJogadas++;
    return 1;
}

int registrarAposta(RegistroRodada *r, EventoAposta evento) {
    if (r == NULL || r->encerradaPorCorrida || r->qtdEventos < 0 || r->qtdEventos >= VAR_MAX_EVENTOS || 
        evento.indiceJogador < 0 || evento.indiceJogador >= VAR_MAX_JOGADAS || (evento.dupla != 1 && evento.dupla != 2) ||
        evento.tipo < VAR_PEDIU_TRUCO || evento.tipo > VAR_CORREU) {
        return 0;
    }
    r->eventos[r->qtdEventos] = evento;
    r->qtdEventos++;

    if (evento.tipo == VAR_CORREU) {
        r->encerradaPorCorrida = 1;
    }
    return 1;
}

// Navegação pela Lista [fazer uso do '?' substitui o if-else]
const tp_no *obterPrimeiraRodadaVar(const tp_listad *lista) {
    return lista == NULL ? NULL : lista->ini;
}
const tp_no *obterUltimaRodadaVar(const tp_listad *lista) {
    return lista == NULL ? NULL : lista->fim;
}
const tp_no *avancarVar(const tp_no *atual) {
    return atual == NULL ? NULL : atual->prox;
}
const tp_no *voltarVar(const tp_no *atual) {
    return atual == NULL ? NULL : atual->ant;
}

//Exibição dos registros
static const char *obterDescricaoEvento(TipoEventoAposta tipo) {
    switch (tipo) {
        case VAR_PEDIU_TRUCO: return "pediu TRUCO";
        case VAR_PEDIU_SEIS:  return "pediu SEIS";
        case VAR_PEDIU_DOZE:  return "pediu DOZE";
        case VAR_ACEITOU:     return "bancou a aposta";
        case VAR_CORREU:      return "CORREU";
        default:              return "evento desconhecido";
    }
}

void exibirRodada(const RegistroRodada *r, const Jogador jogadores[4]) {
    if (r == NULL || jogadores == NULL) return;

    printf("\n========== VAR: MAO %d / RODADA %d ==========\n", r->numeroMao, r->numeroRodada);
    printf("Vira: ");
    carta_imprimir(r->vira);
    printf("\nManilha: %s\n", valor_texto(r->manilha_da_vira));

    printf("\nJogadas (%d):\n", r->qtdJogadas);
    for (int i = 0; i < r->qtdJogadas && i < VAR_MAX_JOGADAS; i++) {
        int indice = r->indicesJogadores[i];
        if (indice < 0 || indice >= VAR_MAX_JOGADAS) {
            printf("  Jogador invalido no registro.\n");
            continue;
        }
        printf("  %s (Dupla %d) jogou ", jogadores[indice].nome, jogadores[indice].dupla);
        carta_imprimir(r->cartas[i]);
        printf("\n");
    }

    printf("\nNegociacao de apostas (%d evento(s)):\n", r->qtdEventos);
    if (r->qtdEventos == 0) {
        printf("  Nenhum pedido registrado.\n");
    }
    for (int i = 0; i < r->qtdEventos && i < VAR_MAX_EVENTOS; i++) {
        EventoAposta evento = r->eventos[i];
        if (evento.indiceJogador < 0 || evento.indiceJogador >= VAR_MAX_JOGADAS) {
            printf("  Evento com jogador invalido.\n");
            continue;
        }
        printf("  %s (Dupla %d) %s", jogadores[evento.indiceJogador].nome, evento.dupla, obterDescricaoEvento(evento.tipo));
        printf(" | valor anterior: %d", evento.valorAnterior);
        if (evento.valorSolicitado != 0) {
            printf(" | valor solicitado: %d", evento.valorSolicitado);
        }
        printf("\n");
    }
    if (r->encerradaPorCorrida)
        printf("\nRodada interrompida: uma dupla correu da aposta.\n");
    if (r->duplaVencedora == -1) {
        printf("Resultado: ainda nao definido.\n");
    } else if (r->duplaVencedora == 0) {
        printf("Resultado: empate entre as duplas.\n");
    } else {
        printf("Resultado: Dupla %d venceu.\n", r->duplaVencedora);
    }
    printf("Valor confirmado da mao: %d ponto(s).\n", r->valorMao);
}

void exibirHistorico(const tp_listad *lista, int ordem, const Jogador jogadores[4]) {
    if (lista == NULL) {
        printf("Lista nao inicializada.\n");
        return;
    }
    if (verificarHistoricoVazio(lista)) {
        printf("VAR vazio: nenhuma rodada registrada.\n");
        return;
    }
    if (jogadores == NULL) return;

    if (ordem == 1) {
        for (const tp_no *atual = lista->ini; atual != NULL; atual = atual->prox) {
            exibirRodada(&atual->info, jogadores);
        }
    } else if (ordem == 2) {
        for (const tp_no *atual = lista->fim; atual != NULL; atual = atual->ant) {
            exibirRodada(&atual->info, jogadores);
        }
    } else {
        printf("Ordem invalida. Digite 1 (direta) ou 2 (inversa).\n");
    }
}

//Parte Interativa
void consultarVar(const tp_listad *lista, const Jogador jogadores[4]) {
    if (verificarHistoricoVazio(lista) || jogadores == NULL) {
        printf("VAR vazio: nenhuma rodada para consultar.\n");
        return;
    }
    const tp_no *atual = obterPrimeiraRodadaVar(lista);
    char opcao[32];

    while (atual != NULL) {
        exibirRodada(&atual->info, jogadores);
        printf("\n[P] Proxima  [A] Anterior  [I] Inicio  [F] Fim  [S] Sair: ");
        if (fgets(opcao, sizeof(opcao), stdin) == NULL) return;

        if (opcao[0] == '\n' || opcao[0] == '\0') continue;

        switch (tolower((unsigned char)opcao[0])) {
            case 'p':
                if (avancarVar(atual) != NULL) atual = avancarVar(atual);
                else printf("Ja esta na ultima rodada.\n");
                break;
            case 'a':
                if (voltarVar(atual) != NULL) atual = voltarVar(atual);
                else printf("Ja esta na primeira rodada.\n");
                break;
            case 'i':
                atual = obterPrimeiraRodadaVar(lista);
                break;
            case 'f':
                atual = obterUltimaRodadaVar(lista);
                break;
            case 's':
                return;
            default:
                printf("Opcao invalida.\n");
                break;
        }
    }
}