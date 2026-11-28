#include "salvamento.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define VERSAO_SALVAMENTO 1
static const unsigned char assinatura[9] = {'T', 'R', 'U', 'C', 'O', 'S', 'A', 'V', 'E'};

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

static int gravarCarta(FILE *arquivo, Carta carta) {
    return gravarI32(arquivo, (int)carta.naipe) && gravarI32(arquivo, (int)carta.valor);
}

static int lerCarta(FILE *arquivo, Carta *carta) {
    int naipe, valor;
    if (!lerI32(arquivo, &naipe) || !lerI32(arquivo, &valor) ||
        naipe < OUROS || naipe >= NUM_NAIPES || valor < QUATRO || valor >= NUM_VALORES)
        return 0;
    carta->naipe = (Naipe)naipe;
    carta->valor = (Valor)valor;
    return 1;
}

static int gravarCabecalho(FILE *arquivo, int andamento) {
    return fwrite(assinatura, sizeof(assinatura), 1, arquivo) == 1 &&
           gravarI32(arquivo, VERSAO_SALVAMENTO) && gravarI32(arquivo, andamento);
}

static int lerCabecalho(FILE *arquivo, int *andamento) {
    unsigned char lida[sizeof(assinatura)];
    int versao;
    if (fread(lida, sizeof(lida), 1, arquivo) != 1 || memcmp(lida, assinatura, sizeof(lida)) != 0 ||
        !lerI32(arquivo, &versao) || versao != VERSAO_SALVAMENTO ||
        !lerI32(arquivo, andamento) || (*andamento != 0 && *andamento != 1)) return 0;
    return 1;
}

int salvamento_disponivel(const char *caminho) {
    int andamento = 0;
    FILE *arquivo;
    if (caminho == NULL) return 0;
    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) return 0;
    int valido = lerCabecalho(arquivo, &andamento);
    fclose(arquivo);
    return valido && andamento;
}

static int gravarMesa(FILE *arquivo, const Mesa *mesa) {
    int assentos[NUM_JOGADORES];
    int quantidade = 0;
    Ordem ordem;
    Jogador *jogador;

    if (mesa->pontosDupla1 < 0 || mesa->pontosDupla1 > PONTOS_PARA_VENCER || mesa->pontosDupla2 < 0 || mesa->pontosDupla2 > PONTOS_PARA_VENCER ||
        mesa->numeroDaRodada < 1 || mesa->inicioDaRodada < -1 || mesa->inicioDaRodada >= NUM_JOGADORES ||
        !gravarI32(arquivo, mesa->pontosDupla1) || !gravarI32(arquivo, mesa->pontosDupla2) ||
        !gravarI32(arquivo, mesa->numeroDaRodada) || !gravarI32(arquivo, mesa->inicioDaRodada) || !gravarCarta(arquivo, mesa->vira)) return 0;

    for (int i = 0; i < NUM_JOGADORES; i++) {
        const Jogador *j = &mesa->jogadores[i];
        size_t tamanho = strlen(j->nome);
        if (tamanho == 0 || tamanho >= sizeof(j->nome) || j->assento != i || j->dupla != (i % 2) + 1 || fwrite(j->nome, sizeof(j->nome), 1, arquivo) != 1 ||
            !gravarI32(arquivo, j->dupla) || !gravarI32(arquivo, j->assento)) return 0;
    }

    ordem = mesa->ordem;
    while (removerMesa(&ordem, &jogador)) {
        int assento = jogador_assento(jogador);
        if (assento < 0 || assento >= NUM_JOGADORES || quantidade >= NUM_JOGADORES) return 0;
        for (int i = 0; i < quantidade; i++) if (assentos[i] == assento) return 0;
        assentos[quantidade++] = assento;
    }
    if (quantidade != NUM_JOGADORES || !gravarI32(arquivo, quantidade)) return 0;
    for (int i = 0; i < quantidade; i++) if (!gravarI32(arquivo, assentos[i])) return 0;
    return 1;
}

static int lerMesa(FILE *arquivo, Mesa *mesa) {
    int quantidade, assentos[NUM_JOGADORES];
    if (!lerI32(arquivo, &mesa->pontosDupla1) || !lerI32(arquivo, &mesa->pontosDupla2) ||
        !lerI32(arquivo, &mesa->numeroDaRodada) || !lerI32(arquivo, &mesa->inicioDaRodada) || !lerCarta(arquivo, &mesa->vira) || mesa->pontosDupla1 < 0 ||
        mesa->pontosDupla1 > PONTOS_PARA_VENCER || mesa->pontosDupla2 < 0 || mesa->pontosDupla2 > PONTOS_PARA_VENCER || mesa->numeroDaRodada < 1 ||
        mesa->inicioDaRodada < -1 || mesa->inicioDaRodada >= NUM_JOGADORES) return 0;

    for (int i = 0; i < NUM_JOGADORES; i++) {
        Jogador *j = &mesa->jogadores[i];
        if (fread(j->nome, sizeof(j->nome), 1, arquivo) != 1 || !memchr(j->nome, '\0', sizeof(j->nome)) || !lerI32(arquivo, &j->dupla) ||
            !lerI32(arquivo, &j->assento) || j->nome[0] == '\0' || j->assento != i || j->dupla != (i % 2) + 1) return 0;
        j->qtdCartas = 0;
    }

    if (!lerI32(arquivo, &quantidade) || quantidade != NUM_JOGADORES) return 0;
    for (int i = 0; i < quantidade; i++) {
        if (!lerI32(arquivo, &assentos[i]) || assentos[i] < 0 || assentos[i] >= NUM_JOGADORES)
            return 0;
        for (int j = 0; j < i; j++) if (assentos[j] == assentos[i]) return 0;
    }

    inicializarMesa(&mesa->ordem);
    for (int i = 0; i < quantidade; i++)
        if (!inserirMesa(&mesa->ordem, &mesa->jogadores[assentos[i]])) return 0;
    return 1;
}

int salvarPartida(const char *caminho, const Mesa *mesa, const tp_listad *historico) {
    char temporario[512];
    FILE *arquivo;
    int sucesso;
    if (caminho == NULL || mesa == NULL || historico == NULL ||
        strlen(caminho) + sizeof(".tmp") > sizeof(temporario)) return 0;
    strcpy(temporario, caminho);
    strcat(temporario, ".tmp");
    arquivo = fopen(temporario, "wb");
    if (arquivo == NULL) return 0;

    sucesso = gravarCabecalho(arquivo, historico->partidaEncerrada ? 0 : 1) &&
              gravarMesa(arquivo, mesa) && gravarHistorico(arquivo, historico);
    if (fclose(arquivo) != 0) sucesso = 0;
    if (!sucesso) {
        remove(temporario);
        return 0;
    }

    // 'rename' substitui o destino em POSIX; remove antes também atende ao Windows
    remove(caminho);
    if (rename(temporario, caminho) != 0) {
        remove(temporario);
        return 0;
    }
    return 1;
}

int carregarPartida(const char *caminho, Mesa *mesa, tp_listad **historico) {
    FILE *arquivo;
    int andamento;
    tp_listad *lista;

    if (caminho == NULL || mesa == NULL || historico == NULL) return 0;
    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) return 0;
    if (!lerCabecalho(arquivo, &andamento) || !andamento || !lerMesa(arquivo, mesa)) {
        fclose(arquivo);
        return 0;
    }
    lista = lerHistorico(arquivo);
    if (fclose(arquivo) != 0 || lista == NULL || lista->partidaEncerrada) {
        destruirHistorico(lista);
        return 0;
    }
    descartarRegistrosDaMaoEmAndamento(lista, mesa->numeroDaRodada);
    *historico = lista;
    return 1;
}