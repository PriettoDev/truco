#ifndef HISTORICO_H
#define HISTORICO_H

#include "jogador.h"

#define VAR_MAX_JOGADAS 4
#define VAR_MAX_EVENTOS 8

typedef enum{ //Criando apelidos necessários - tipo enumerado
    VAR_PEDIU_TRUCO,
    VAR_PEDIU_SEIS,    
    VAR_PEDIU_DOZE,
    VAR_ACEITOU,
    VAR_CORREU
} TipoEventoAposta;

typedef struct {
    TipoEventoAposta tipo;
    int indiceJogador;  // 0 a 3, corresponde a mesa.jogadores[]
    int dupla;  // 1 ou 2
    int valorAnterior;  // valor confirmado antes do pedido/resposta
    int valorSolicitado;  // 3, 6 ou 12 no pedido; 0 se não se aplicar
} EventoAposta;

//Cada rodada será um tp_item da lista duplamente encadeada
typedef struct {
    int numeroMao;
    int numeroRodada;
    Carta vira;
    Valor manilha_da_vira;
    int indicesJogadores[VAR_MAX_JOGADAS];    
    Carta cartas[VAR_MAX_JOGADAS];
    int qtdJogadas; // 0 a 4, podendo ter uma jogada interrompida por corrida
    EventoAposta eventos[VAR_MAX_EVENTOS];
    int qtdEventos;
    int duplaVencedora;
    int valorMao;
    int encerradaPorCorrida;
} RegistroRodada;

typedef RegistroRodada tp_item;

typedef struct tp_no_aux {
    struct tp_no_aux *ant;
    tp_item info;
    struct tp_no_aux *prox;     
} tp_no;

typedef struct {
    tp_no *ini;
    tp_no *fim;
    int tamanho;
    int partidaEncerrada;
} tp_listad;

//Gerências da lista duplamente encadeada
tp_listad *inicializarHistorico(void);
int verificarHistoricoVazio(const tp_listad *lista);
int adicionarRodadaHistorico(tp_listad *lista, tp_item registro);
const tp_no *buscarRodadaHistorico(const tp_listad *lista, int mao, int rodada);
void exibirHistorico(const tp_listad *lista, int ordem, const Jogador jogadores[4]);
int finalizarPartidaHistorico(tp_listad *lista);
tp_listad *destruirHistorico(tp_listad *lista);

//feito só para testes (PRECISO DO REGISTRO DA RODADA [JOGADA E APOSTA])============
RegistroRodada criarRegistroRodada(int mao, int rodada, Carta vira, int valorMao);
int registrarJogada(RegistroRodada *registro, int indiceJogador, Carta carta);
int registrarAposta(RegistroRodada *registro, EventoAposta evento);
//==================================================================================

//Navegação do VAR: vai avançar e voltar um registro por vez.
const tp_no *obterPrimeiraRodadaVar(const tp_listad *lista);
const tp_no *obterUltimaRodadaVar(const tp_listad *lista);
const tp_no *avancarVar(const tp_no *atual);
const tp_no *voltarVar(const tp_no *atual);
void exibirRodada(const RegistroRodada *registro, const Jogador jogadores[4]);
void consultarVar(const tp_listad *lista, const Jogador jogadores[4]);

#endif