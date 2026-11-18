#ifndef PARTIDA_GUI_H
#define PARTIDA_GUI_H

/*
 * partida_gui.h - estado da partida no modo passa a vez, separado do desenho.
 *
 * Aqui nao entra raylib: so os modulos de src/ e a maquina de estados. Assim
 * a partida inteira pode ser testada sem abrir janela, e a tela so le o
 * estado e avisa o que o jogador fez (confirmar ou jogar uma carta).
 * O diagrama dos estados esta no topo de partida_gui.c.
 */
#include "mesa.h"
#include "baralho.h"
#include "mao.h"

/* Tempos em segundos. O voo e a apuracao sao do estado, e nao so da
   animacao, porque ate eles acabarem nenhum clique pode ser aceito. */
#define PARTIDA_TEMPO_VOO      0.35f
#define PARTIDA_TEMPO_APURACAO 1.5f
#define PARTIDA_TEMPO_TRAVA    0.3f   /* clique duplo nao atravessa a tela */
#define PARTIDA_TEMPO_ANUNCIO  1.2f   /* "TRUCO!" na tela antes da resposta */

typedef enum {
    PARTIDA_INICIO_MAO,
    PARTIDA_COBERTURA,
    PARTIDA_VEZ_DO_JOGADOR,
    PARTIDA_CARTA_VOANDO,
    PARTIDA_APURANDO,
    PARTIDA_FIM_MAO,
    PARTIDA_FIM_PARTIDA,
    /* Truco (no fim para os numeros dos estados antigos nao mudarem). */
    PARTIDA_ANUNCIO_PEDIDO,      /* "TRUCO!" com o nome de quem pediu */
    PARTIDA_COBERTURA_RESPOSTA,  /* passa o computador para quem responde */
    PARTIDA_RESPOSTA             /* aceitar, correr ou pedir mais */
} EstadoPartida;

typedef enum { RESPOSTA_ACEITAR, RESPOSTA_CORRER, RESPOSTA_AUMENTAR } RespostaTruco;

/* Ultimo acontecimento da negociacao, para quem vem depois (animacao dos
   personagens, sons e VAR) reagir sem a partida conhecer nenhum deles.
   valor: em PEDIU e AUMENTOU, o valor pedido; em ACEITOU, o valor que a
   mao passou a valer; em CORREU, os pontos que a outra dupla levou. */
typedef enum {
    EVENTO_NENHUM, EVENTO_PEDIU, EVENTO_ACEITOU, EVENTO_CORREU, EVENTO_AUMENTOU
} TipoEvento;

typedef struct {
    TipoEvento tipo;
    int assento;   /* quem fez */
    int valor;
} EventoPartida;

/* Carta ja jogada nesta rodada. Guarda o assento para a tela saber de que
   lado da mesa ela fica (gira junto com a camera) e a posicao que ocupava
   na mao, de onde o voo comeca. */
typedef struct {
    Carta carta;
    int assento;
    int posicaoNaMao;
} CartaNaMesa;

/* ATENCAO: nunca copie um PartidaGui por valor. mesa.ordem guarda ponteiros
   para mesa.jogadores; numa copia eles continuariam apontando para o
   original. Use sempre por ponteiro e reinicie no mesmo lugar. */
typedef struct {
    Mesa mesa;               /* jogadores, ordem de jogo, vira e placar */
    Baralho baralho;         /* embaralhado de novo a cada mao */
    Mao mao;                 /* mao atual: rodadas, aposta e manilha */
    Valor manilha;
    Ordem fila;              /* copia de mesa.ordem consumida na rodada */

    CartaNaMesa naMesa[TOTAL_CARTAS];
    int qtdNaMesa;

    /* Apuracao da ultima rodada: indices em naMesa das cartas destacadas
       (-1 = nenhuma) e o resultado (0, 1 ou EMPATE). Na vitoria so destaque[0]
       e usado; no empate, a melhor carta de cada dupla. */
    int destaque[2];
    int resultadoRodada;

    int assentoDaVez;        /* quem joga; nas telas de fim, o ultimo que jogou */
    int duplaVencedoraMao;   /* 0, 1 ou MAO_EMPATE, valido em FIM_MAO */
    int pontosDaMao;
    int duplaVencedoraPartida;   /* 0 ou 1, valido em FIM_PARTIDA */

    /* Truco: quem fez o ultimo pedido e quem responde agora (o proximo na
       ordem de jogo depois dele, sempre da outra dupla). A vez de jogar
       continua com assentoDaVez durante toda a negociacao (main.c 386-404). */
    int assentoPediu;
    int assentoResponde;
    int assentoCorreu;       /* -1, ou quem correu (a mao acabou por corrida) */

    /* Le-se comparando contadorEventos com o ultimo valor visto: muda a
       cada evento e nunca volta a zero, nem numa partida nova. */
    EventoPartida ultimoEvento;
    unsigned contadorEventos;

    EstadoPartida estado;
    float tempoNoEstado;
    int falhas;              /* quantas funcoes de src/ devolveram erro */
} PartidaGui;

/* Comeca uma partida nova do zero (placar 0 x 0). */
void partida_iniciar(PartidaGui *p);

/* Avanca o tempo e as transicoes automaticas. Chamar uma vez por quadro. */
void partida_atualizar(PartidaGui *p, float dt);

/* Clique ou ESPACO. Devolve 1 se foi aceito (mudou de estado). */
int partida_confirmar(PartidaGui *p);

/* O jogador da vez joga a carta da posicao pos (0 a qtdCartas-1).
   Devolve 1 se a carta saiu da mao. */
int partida_jogar_carta(PartidaGui *p, int pos);

/* Quem esta na vez pede truco (ou seis, nove, doze...). So em
   VEZ_DO_JOGADOR, antes de jogar a carta e depois da trava. Devolve 1 se
   o modulo aceitou o pedido. */
int partida_pedir_truco(PartidaGui *p);

/* Quem responde escolhe, em RESPOSTA e depois da trava. Devolve 1 se o
   modulo aceitou. */
int partida_responder(PartidaGui *p, RespostaTruco resposta);

/* O botao de pedir esta ativo agora? Pergunta ao proprio modulo, numa copia
   da Mao, em vez de repetir as regras do aposta.c. Em *proximo vai o valor
   que seria pedido (pode ser NULL). Vale para quem esta na vez (em
   VEZ_DO_JOGADOR) e para quem responde (em RESPOSTA, o "pedir mais"). */
int partida_pode_pedir(const PartidaGui *p, int *proximo);

/* Proximo valor que alguma dupla poderia pedir agora (0 = nenhum, a mao
   ja esta no maximo). So para o rotulo do botao desativado: "PEDIR SEIS"
   cinza mostra que o seis existe, mas nao e a sua vez de pedir. */
int partida_proximo_valor(const PartidaGui *p);

/* Nome do valor para botoes e anuncios: 3 TRUCO, 6 SEIS, 9 NOVE, 12 DOZE.
   Fora da tabela, o proprio numero ("15"), para a tela nao quebrar se a
   sequencia do aposta.c mudar. saida precisa de pelo menos 12 bytes. */
void partida_nome_do_valor(int valor, char *saida, int tamanho);

/* Consultas para a tela. */
int partida_assento_visto(const PartidaGui *p);   /* de quem e a camera */
int partida_mao_visivel(const PartidaGui *p);     /* assento cuja mao pode aparecer, ou -1 */
const char *partida_nome(int assento);
int partida_pontos(const PartidaGui *p, int dupla); /* dupla interna 0/1 */

#endif
