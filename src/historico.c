#include "historico.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//Histórico VAR - Lista Duplamente Encadeada: cada NO armazena uma cópia de uma rodada.

// Adaptacao direta da inicializa_listad
tp_listad *inicializarHistorico(void) {
    tp_listad *lista = malloc(sizeof(*lista));
    if (lista == NULL)
        return NULL;

    lista->ini = NULL;
    lista->fim = NULL;
    lista->tamanho = 0;
    lista->partidaEncerrada = 0;
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

//Ao retomar ao jogo, a partida volta ao início da próxima mão não concluída
//Registros dessa mão são descartados para não duplicá-los ao jogá-la novamente
void descartarRegistrosDaMaoEmAndamento(tp_listad *lista, int proximaMao) {
    if (lista == NULL) return;
    while (lista->fim != NULL && lista->fim->info.numeroMao >= proximaMao) {
        tp_no *remover = lista->fim;
        lista->fim = remover->ant;
        if (lista->fim != NULL) lista->fim->prox = NULL;
        else lista->ini = NULL;
        free(remover);
        lista->tamanho--;
    }
}

//Cada inteiro é salvo como int32_t, não como struct
static int gravarI32(FILE *arquivo, int valor) {
    int32_t salvo = (int32_t)valor;
    return fwrite(&salvo, sizeof(salvo), 1, arquivo) == 1;
}

static int lerI32(FILE *arquivo, int *valor) {
    int32_t lido;
    if (fread(&lido, sizeof(lido), 1, arquivo) != 1) return 0;
    *valor = (int)lido;
    return 1;
}

static int cartaValida(Carta carta) {
    return carta.naipe >= OUROS && carta.naipe < NUM_NAIPES &&
           carta.valor >= QUATRO && carta.valor < NUM_VALORES;
}

static int gravarCarta(FILE *arquivo, Carta carta) {
    return gravarI32(arquivo, (int)carta.naipe) &&
           gravarI32(arquivo, (int)carta.valor);
}

static int lerCarta(FILE *arquivo, Carta *carta) {
    int naipe, valor;
    if (!lerI32(arquivo, &naipe) || !lerI32(arquivo, &valor)) return 0;
    carta->naipe = (Naipe)naipe;
    carta->valor = (Valor)valor;
    return cartaValida(*carta);
}

static int registroValido(const RegistroRodada *r) {
    if (r->numeroMao < 1 || r->numeroRodada < 1 ||
        r->qtdJogadas < 0 || r->qtdJogadas > VAR_MAX_JOGADAS ||
        r->qtdEventos < 0 || r->qtdEventos > VAR_MAX_EVENTOS ||
        r->duplaVencedora < -1 || r->duplaVencedora > 2 ||
        r->valorMao < 0 || r->encerradaPorCorrida < 0 || r->encerradaPorCorrida > 1 ||
        !cartaValida(r->vira) || r->manilha_da_vira < QUATRO || r->manilha_da_vira >= NUM_VALORES)
        return 0;
    for (int i = 0; i < r->qtdJogadas; i++) {
        if (r->indicesJogadores[i] < 0 || r->indicesJogadores[i] >= VAR_MAX_JOGADAS ||
            !cartaValida(r->cartas[i])) return 0;
    }
    for (int i = 0; i < r->qtdEventos; i++) {
        const EventoAposta *e = &r->eventos[i];
        if (e->tipo < VAR_PEDIU_TRUCO || e->tipo > VAR_CORREU ||
            e->indiceJogador < 0 || e->indiceJogador >= VAR_MAX_JOGADAS ||
            (e->dupla != 1 && e->dupla != 2)) return 0;
    }
    return 1;
}

static int gravarRegistro(FILE *arquivo, const RegistroRodada *r) {
    if (!registroValido(r) ||
        !gravarI32(arquivo, r->numeroMao) || !gravarI32(arquivo, r->numeroRodada) ||
        !gravarCarta(arquivo, r->vira) || !gravarI32(arquivo, (int)r->manilha_da_vira) ||
        !gravarI32(arquivo, r->qtdJogadas)) return 0;
    for (int i = 0; i < r->qtdJogadas; i++)
        if (!gravarI32(arquivo, r->indicesJogadores[i]) || !gravarCarta(arquivo, r->cartas[i])) return 0;
    if (!gravarI32(arquivo, r->qtdEventos)) return 0;
    for (int i = 0; i < r->qtdEventos; i++) {
        const EventoAposta *e = &r->eventos[i];
        if (!gravarI32(arquivo, (int)e->tipo) || !gravarI32(arquivo, e->indiceJogador) ||
            !gravarI32(arquivo, e->dupla) || !gravarI32(arquivo, e->valorAnterior) ||
            !gravarI32(arquivo, e->valorSolicitado)) return 0;
    }
    return gravarI32(arquivo, r->duplaVencedora) &&
           gravarI32(arquivo, r->valorMao) && gravarI32(arquivo, r->encerradaPorCorrida);
}

static int lerRegistro(FILE *arquivo, RegistroRodada *r) {
    int manilha, tipo;
    memset(r, 0, sizeof(*r));
    if (!lerI32(arquivo, &r->numeroMao) || !lerI32(arquivo, &r->numeroRodada) ||
        !lerCarta(arquivo, &r->vira) || !lerI32(arquivo, &manilha) ||
        manilha < QUATRO || manilha >= NUM_VALORES || !lerI32(arquivo, &r->qtdJogadas) ||
        r->qtdJogadas < 0 || r->qtdJogadas > VAR_MAX_JOGADAS) return 0;
    r->manilha_da_vira = (Valor)manilha;
    for (int i = 0; i < r->qtdJogadas; i++)
        if (!lerI32(arquivo, &r->indicesJogadores[i]) ||
            !lerCarta(arquivo, &r->cartas[i])) return 0;
    if (!lerI32(arquivo, &r->qtdEventos) || r->qtdEventos < 0 ||
        r->qtdEventos > VAR_MAX_EVENTOS) return 0;
    for (int i = 0; i < r->qtdEventos; i++) {
        EventoAposta *e = &r->eventos[i];
        if (!lerI32(arquivo, &tipo) || !lerI32(arquivo, &e->indiceJogador) ||
            !lerI32(arquivo, &e->dupla) || !lerI32(arquivo, &e->valorAnterior) ||
            !lerI32(arquivo, &e->valorSolicitado)) return 0;
        e->tipo = (TipoEventoAposta)tipo;
    }
    if (!lerI32(arquivo, &r->duplaVencedora) || !lerI32(arquivo, &r->valorMao) ||
        !lerI32(arquivo, &r->encerradaPorCorrida)) return 0;
    return registroValido(r);
}

int gravarHistorico(FILE *arquivo, const tp_listad *lista) {
    if (arquivo == NULL || lista == NULL || lista->tamanho < 0 ||
        !gravarI32(arquivo, lista->tamanho) || !gravarI32(arquivo, lista->partidaEncerrada != 0))
        return 0;
    int gravados = 0;
    for (const tp_no *no = lista->ini; no != NULL; no = no->prox) {
        if (!gravarRegistro(arquivo, &no->info)) return 0;
        gravados++;
    }
    return gravados == lista->tamanho;
}

tp_listad *lerHistorico(FILE *arquivo) {
    int quantidade, encerrada;
    if (arquivo == NULL || !lerI32(arquivo, &quantidade) || quantidade < 0 || quantidade > 100000 ||
        !lerI32(arquivo, &encerrada) || (encerrada != 0 && encerrada != 1)) return NULL;
    tp_listad *lista = inicializarHistorico();
    if (lista == NULL) return NULL;
    for (int i = 0; i < quantidade; i++) {
        RegistroRodada registro;
        if (!lerRegistro(arquivo, &registro) || !adicionarRodadaHistorico(lista, registro)) {
            destruirHistorico(lista);
            return NULL;
        }
    }
    lista->partidaEncerrada = encerrada;
    return lista;
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