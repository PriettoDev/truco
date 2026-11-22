#include <stdio.h>
#include <stdlib.h>   // srand
#include <time.h>     // time
#include <ctype.h>    // manipulação de caracteres
#include <errno.h>    // tratamento de erros do sistema
#include <limits.h>   // definição de limites de tipos de dados
#include <string.h>

#include "mesa.h"
#include "mao.h"
#include "baralho.h"
#include "historico.h"

#ifdef _WIN32
#include <windows.h>
#endif

/*
 *  Truco no Bar do Pato - Integracao das Unidades 1 e 2
 *  Os modulos existentes cuidam das regras. A main.c cuida das entradas, sequencia
 * de jogo, negociacoes, pontuacao e gravacao do VAR em memoria.
 * Nao ha jogadas automaticas: todas as escolhas sao feitas pelos jogadores 
 */

typedef enum {
    JOGO_ERRO = 0,
    JOGO_OK = 1,
    JOGO_SAIR = 2
} ResultadoFluxo;

/* ------------------------------------------------------------------ */
/* ENTRADA SEGURA                                                     */
/* ------------------------------------------------------------------ */

//Le uma linha completa
static int lerLinha(char *destino, size_t capacidade) {
    size_t n;
    int ch;

    if (destino == NULL || capacidade < 2) return 0;
    if (fgets(destino, (int)capacidade, stdin) == NULL) return 0;

    n = strlen(destino);
    if (n > 0 && destino[n - 1] == '\n') {
        destino[n - 1] = '\0';
    } else if (!feof(stdin)) {
        do {
            ch = getchar();
        } while (ch != '\n' && ch != EOF);
        puts("Entrada muito longa. Tente novamente.");
        return -1;
    }
    return 1;
}

static int lerNumero(const char *mensagem, int minimo, int maximo, int *valor) {
    char linha[128];
    char *fim;
    long numero;
    int leitura;

    if (valor == NULL) return 0;
    for (;;) {
        printf("%s", mensagem);
        fflush(stdout);
        leitura = lerLinha(linha, sizeof(linha));
        if (leitura == 0) return 0;
        if (leitura < 0) continue;

        errno = 0;
        numero = strtol(linha, &fim, 10);
        while (isspace((unsigned char)*fim)) fim++;
        if (fim == linha || *fim != '\0' || errno == ERANGE || numero < minimo || numero > maximo || 
        numero > INT_MAX) {
            printf("Opcao invalida. Digite um numero de %d e %d.\n", minimo, maximo);
            continue;
        }
        *valor = (int)numero;
        return 1;
    }


}

static void removerEspacosDasPontas(char *texto) {
    char *inicio = texto;
    size_t tamanho;

    while (isspace((unsigned char)*inicio)) inicio++;
    if (inicio != texto) memmove(texto, inicio, strlen(inicio) + 1);
    tamanho = strlen(texto);
    while (tamanho > 0 && isspace((unsigned char)texto[tamanho - 1])) {
        texto[--tamanho] = '\0';
    }
}

static int nomesIguais(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

/* ------------------------------------------------------------------ */
/* PREPARACAO DO JOGO                                                 */
/* ------------------------------------------------------------------ */

// Cria a mesa, os 4 jogadores e coloca todos na lista circular.
static int configurarMesa(Mesa *mesa) {
    char nome[128];
    inicializarMesa(&mesa->ordem);    // fila circular vazia
    mesa->numeroDaRodada = 1;
    mesa->inicioDaRodada = -1;
    mesa->pontosDupla1 = 0;
    mesa->pontosDupla2 = 0;

    puts("\n========== TRUCO NO BAR DO PATO ==========");
    puts("Cadastro: os assentos 1 e 3 sao da Dupla 1;");
    puts("          os assentos 2 e 4 sao da Dupla 2;");

    for (int i = 0; i < NUM_JOGADORES; i++) {
        int repetido;
        for (;;) {
            int leitura;
            printf("Nome do Jogador %d (Dupla %d): ", i+1, i%2+1);
            fflush(stdout);
            leitura = lerLinha(nome, sizeof(nome));
            if (leitura == 0) return 0;
            if (leitura < 0) continue;
            removerEspacosDasPontas(nome);
            if (nome[0] == '\0' || strlen(nome) >= sizeof(mesa->jogadores[i].nome)) {
                puts("Nome invalido: use de 1 a 29 caracteres.");
                continue;
            }
            repetido = 0;
            for (int j = 0; j < i; j++) {
                if (nomesIguais(nome, mesa->jogadores[j].nome)) repetido = 1;
            }
            if (repetido) {
                puts("Esse nome ja esta cadastrado nesta mesa.");
                continue;
            }
            break;
        }
        mesa->jogadores[i] = jogador_criar(nome, i % 2 + 1, i);
        if (!inserirMesa(&mesa->ordem, &mesa->jogadores[i])) {
            puts("Erro ao inserir jogador na fila circular.");
            return 0;
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* DISTRIBUIÇÃO DAS CARTAS                                            */
/* ------------------------------------------------------------------ */

static int distribuirCartas(Mesa *mesa, Valor *manilha) {
    Baralho baralho;
    Carta c1, c2, c3;
    Ordem ordem = mesa->ordem;
    Jogador *jogador;

    inicializaBaralho(&baralho);
    criaBaralho(&baralho);
    embaralhaBaralho(&baralho);

    while (removerMesa(&ordem, &jogador)) {
        if (!retiraCarta(&baralho, &c1) || !retiraCarta(&baralho, &c2) || !retiraCarta(&baralho, &c3))
            return 0;
        jogador_distribuir_cartas(jogador, c1, c2, c3);
    }

    if (!retiraCarta(&baralho, &mesa->vira)) {
        printf("Erro ao virar a carta.\n");
        return 0;
    }
    *manilha = manilha_da_vira(mesa->vira);

    printf("Vira: ");
    carta_imprimir(mesa->vira);
    printf("  |  Manilha: %s\n", valor_texto(*manilha));

    return 1;
}

/* ------------------------------------------------------------------ */
/* INTERFACE E HISTORICO VAR                                          */
/* ------------------------------------------------------------------ */

static void mostrarPlacar(const Mesa *mesa) {
    printf("\nPLACAR | Dupla 1: %d | Dupla 2: %d | Meta:%d\n", mesa->pontosDupla1, mesa->pontosDupla2, PONTOS_PARA_VENCER);
}

static void mostrarCartasJogador(const Jogador *jogador) {
    printf("\nCartas de %s (Dupla %d):\n", jogador->nome, jogador->dupla);
    
    for (int i = 0; i < jogador->qtdCartas; i++) {
        printf("    %d - ", i+1);
        carta_imprimir(jogador->mao[i]);
        putchar('\n');
    }
}

static void mostrarCartasNaMesa(const Rodada *r, const Mesa *mesa) {
    printf("Cartas desta rodada (%d/%d):\n", r->qtd, TOTAL_CARTAS);
    if (r->qtd == 0)
        puts("  Nenhuma carta jogada.");
    for (int i = 0; i < r->qtd; i++){
        const Jogada *j = &r->jogadas[i];
        printf("    %s (Dupla %d): ", mesa->jogadores[j->jogador].nome, j->dupla + 1);
        carta_imprimir(j->carta);
        putchar('\n');
    }
}

static int registrarEvento(RegistroRodada *registro, TipoEventoAposta tipo, int indiceJogador, int valorAnterior, int pedido, const Mesa *mesa) {
    EventoAposta evento;
    evento.tipo = tipo;
    evento.indiceJogador = indiceJogador;
    evento.dupla = mesa->jogadores[indiceJogador].dupla;
    evento.valorAnterior = valorAnterior;
    evento.valorSolicitado = pedido;
    if (!registrarAposta(registro, evento)) {
        puts("Erro: nao foi possivel registrar a negociacao no VAR.");
        return 0;
    }
    return 1;
}

//Um integrante da dupla adversaria responde, sem consumir a vez de jogar
static int escolherRespondente(const Mesa *mesa, int duplaInterna, int *indice) {
    int opcao;
    int primeiro = duplaInterna;       /* Assentos: 0/2 ou 1/3. */
    int segundo = duplaInterna + 2;

    printf("\nDupla %d deve responder. Quem fala pela dupla?\n", duplaInterna + 1);
    printf("  1 - %s\n  2 - %s\n",
           mesa->jogadores[primeiro].nome, mesa->jogadores[segundo].nome);
    if (!lerNumero("Escolha (1-2): ", 1, 2, &opcao)) return 0;
    *indice = (opcao == 1) ? primeiro : segundo;
    return 1;
}

// Mantem os pedidos de 3/6/12 e obriga a resposta da outra dupla.
static ResultadoFluxo resolverAposta(Mesa *mesa, Mao *mao, RegistroRodada *registro) {
    while (aposta_pendente(&mao->aposta) && !mao_terminou(mao)) {
        int respondente, dupla, acao;
        int anterior = mao->aposta.valor;
        int pedido = mao->aposta.pedido;

        dupla = mao->aposta.dupla_responde;
        printf("\n>>> Pedido de %d ponto(s) para a Dupla %d!\n", pedido, dupla + 1);
        if (!escolherRespondente(mesa, dupla, &respondente)) return JOGO_ERRO;

        puts("  1 - Aceitar");
        if (pedido < 12) puts("  2 - Aumentar (seis/doze)");
        puts("  3 - Correr");
        if (!lerNumero("Resposta (1-3): ", 1, 3, &acao)) return JOGO_ERRO;

        if (acao == 1) {
            if (!mao_aceitar(mao, dupla)) return JOGO_ERRO;
            if (!registrarEvento(registro, VAR_ACEITOU, respondente,
                                 anterior, pedido, mesa)) return JOGO_ERRO;
            printf("%s aceitou! A mao vale %d ponto(s).\n",
                   mesa->jogadores[respondente].nome, mao->aposta.valor);
        } else if (acao == 2) {
            int novoPedido;
            TipoEventoAposta tipo;
            if (pedido == 12) {
                puts("Nao e possivel aumentar alem de doze.");
                continue;
            }
            if (!mao_pedir(mao, dupla)) return JOGO_ERRO;
            novoPedido = mao->aposta.pedido;
            tipo = novoPedido == 6 ? VAR_PEDIU_SEIS : VAR_PEDIU_DOZE;
            if (!registrarEvento(registro, tipo, respondente,
                                 pedido, novoPedido, mesa)) return JOGO_ERRO;
            printf("%s aumentou para %d!\n",
                   mesa->jogadores[respondente].nome, novoPedido);
        // A contraproposta confirma o pedido anterior (3 ou 6).
        } else {
            if (!mao_correr(mao, dupla)) return JOGO_ERRO;
            if (!registrarEvento(registro, VAR_CORREU, respondente,
                                 anterior, pedido, mesa)) return JOGO_ERRO;
            printf("%s correu! Dupla %d ganha %d ponto(s).\n",
                   mesa->jogadores[respondente].nome,
                   mao->dupla_vencedora + 1, mao->pontos_ganhos);
        }
    }
    return JOGO_OK;
}

static ResultadoFluxo solicitarTruco(Mesa *mesa, Mao *mao, RegistroRodada *registro) {
    int escolha, indice, dupla, anterior, pedido;

    if (mao->aposta.valor >= 12 || aposta_pendente(&mao->aposta)) {
        puts("Nao e possivel pedir truco neste momento.");
        return JOGO_OK;
    }

    puts("\nQuem deseja pedir truco / aumentar a aposta?");
    for (int i = 0; i < NUM_JOGADORES; i++) {
        printf("  %d - %s (Dupla %d)\n", i + 1, mesa->jogadores[i].nome, mesa->jogadores[i].dupla);
    }
    if (!lerNumero("Jogador (1-4): ", 1, 4, &escolha)) return JOGO_ERRO;

    indice = escolha - 1;
    dupla = indice % 2;
    if (dupla == mao->aposta.ultima_dupla_pediu) {
        puts("Essa dupla fez o ultimo pedido; agora e a vez da adversaria.");
        return JOGO_OK;
    }

    anterior = mao->aposta.valor;
    if (!mao_pedir(mao, dupla)) {
        puts("Pedido invalido pelas regras de aposta.");
        return JOGO_OK;
    }
    pedido = mao->aposta.pedido;
    if (!registrarEvento(registro, pedido == 3 ? VAR_PEDIU_TRUCO :
                         pedido == 6 ? VAR_PEDIU_SEIS : VAR_PEDIU_DOZE,
                         indice, anterior, pedido, mesa)) return JOGO_ERRO;

    printf("\n%s (Dupla %d) pediu %s!\n", mesa->jogadores[indice].nome, dupla + 1,
           pedido == 3 ? "TRUCO" : pedido == 6 ? "SEIS" : "DOZE");

    return resolverAposta(mesa, mao, registro);
}

/* ------------------------------------------------------------------ */
/* JOGADA, RODADA E MAO                                               */
/* ------------------------------------------------------------------ */

// Joga uma carta, removendo-a do vetor do jogador: nao eh automatica
static int jogarCarta(Jogador *jogador, Mao *mao, RegistroRodada *registro) {
    int escolha, indice = jogador_assento(jogador);
    Carta carta, removida;

    mostrarCartasJogador(jogador);
    if (jogador->qtdCartas <= 0) return 0;
    if (!lerNumero("Qual carta jogar? ", 1, jogador->qtdCartas, &escolha)) return 0;

    // Registra na rodada antes de remover, para nao perder a carta se falhar
    carta = jogador->mao[escolha - 1];
    if (!mao_jogar(mao, carta, indice, indice % 2)) return 0;
    if (!jogador_jogar_carta(jogador, escolha - 1, &removida)) return 0;
    if (!registrarJogada(registro, indice, removida)) return 0;

    printf("%s jogou ", jogador->nome);
    carta_imprimir(removida);
    printf(" | Restam %d carta(s).\n", jogador->qtdCartas);
    return 1;
}

// Mostra quem ganhou a mao que acabou de ser apurada.
static ResultadoFluxo realizarRodada(Mesa *mesa, Mao *mao, tp_listad *historico) {
    const int numeroRodada = mao->rodada_atual + 1;
    Ordem fila = mesa->ordem;     /* Remover da copia preserva a ordem real. */
    Jogador *jogador;
    Rodada *rodada = &mao->rodadas[mao->rodada_atual];
    RegistroRodada registro = criarRegistroRodada(mesa->numeroDaRodada, numeroRodada, mesa->vira, mao->aposta.valor);

    printf("\n===== MAO %d / RODADA %d =====\n", mesa->numeroDaRodada, numeroRodada);
    imprimirOrdemDaMao(mesa);

    while (!rodada_completa(rodada) && !mao_terminou(mao)) {
        int opcao;
        ResultadoFluxo resultado;

        if (!removerMesa(&fila, &jogador)) return JOGO_ERRO;
        // O jogador permanece na vez enquanto consulta ou negocia
        for (;;) {
            printf("\n>>> Vez de %s (Dupla %d) <<<\n", jogador->nome, jogador->dupla);
            mostrarCartasNaMesa(rodada, mesa);
            printf("Valor confirmado: %d", mao->aposta.valor);
            if (aposta_pendente(&mao->aposta))
                printf(" | Pedido pendente: %d", mao->aposta.pedido);
            putchar('\n');
            puts("  1 - Jogar uma carta");
            puts("  2 - Pedir truco / aumentar");
            puts("  3 - Consultar VAR das rodadas anteriores");
            puts("  4 - Mostrar placar");
            puts("  0 - Encerrar a partida");
            if (!lerNumero("Opcao: ", 0, 4, &opcao)) return JOGO_ERRO;

            if (opcao == 0) return JOGO_SAIR;
            if (opcao == 3) {
                consultarVar(historico, mesa->jogadores);
                continue;
            }
            if (opcao == 4) {
                mostrarPlacar(mesa);
                continue;
            }
            if (opcao == 2) {
                resultado = solicitarTruco(mesa, mao, &registro);
                if (resultado != JOGO_OK) return resultado;
                if (mao_terminou(mao)) break; // Dupla correu: fim imediato
                continue;
            }
            if (!jogarCarta(jogador, mao, &registro)) {
                puts("Erro ao jogar carta ou entrada encerrada.");
                return JOGO_ERRO;
            }
            break;
        }
    }
    // Atualiza o resultado antes de guardar no VAR
    if (mao_terminou(mao)) {
        registro.duplaVencedora = mao->dupla_vencedora + 1;
        registro.encerradaPorCorrida = 1;
        registro.valorMao = mao->pontos_ganhos;
    } else {
        int n = mao->rodada_atual;
        if (!mao_apurar_rodada(mao)) return JOGO_ERRO;
        registro.duplaVencedora = mao->resultados[n] == EMPATE ? 0 : mao->resultados[n] + 1;
        registro.valorMao = mao->aposta.valor;
        if (registro.duplaVencedora == 0)
            printf("Rodada %d empatada!\n", numeroRodada);
        else
            printf("Rodada %d: Dupla %d venceu!\n",numeroRodada, registro.duplaVencedora);
    }

    if (!adicionarRodadaHistorico(historico, registro)) {
        puts("Erro ao salvar a rodada no VAR.");
        return JOGO_ERRO;
    }

    if (!mao_terminou(mao)) {
        // O vencedor da rodada abre a proxima; no empate, quem abriu mantem.
        if (!definirPrimeiroDaMao(mesa, mao->jogador_abre)) return JOGO_ERRO;
    }
    return JOGO_OK;
}

static void somarPontos(Mesa *mesa, const Mao *mao) {
    int *placar;
    if (mao->dupla_vencedora != 0 && mao->dupla_vencedora != 1) return;
    placar = mao->dupla_vencedora == 0 ? &mesa->pontosDupla1 : &mesa->pontosDupla2;
    *placar += mao->pontos_ganhos;
    if (*placar > PONTOS_PARA_VENCER) *placar = PONTOS_PARA_VENCER;
}

static ResultadoFluxo jogarMao(Mesa *mesa, tp_listad *historico) {
    Mao mao;
    Valor manilha;
    int abre = iniciarNovaRodada(mesa);

    if (abre < 0) return JOGO_ERRO;
    printf("\n===========================================\n");
    printf("MAO %d | Jogador que inicia: %s\n", mesa->numeroDaRodada, mesa->jogadores[abre].nome);
    printf("===========================================\n");

    if (!distribuirCartas(mesa, &manilha)) return JOGO_ERRO;
    mao_iniciar(&mao, manilha, abre);

    while (!mao_terminou(&mao)) {
        ResultadoFluxo resultado = realizarRodada(mesa, &mao, historico);
        if (resultado != JOGO_OK) return resultado;
    }

    if (mao.dupla_vencedora == MAO_EMPATE) {
        puts("\nMao empatada: nenhuma dupla pontuou.");
    } else {
        printf("\nFim da mao %d: Dupla %d ganhou %d ponto(s)!\n", mesa->numeroDaRodada, mao.dupla_vencedora + 1, mao.pontos_ganhos);
    }
    somarPontos(mesa, &mao);
    mostrarPlacar(mesa);
    mesa->numeroDaRodada++;
    return JOGO_OK;
}

/* ------------------------------------------------------------------ */
/* PROGRAMA PRINCIPAL                                                  */
/* ------------------------------------------------------------------ */

int main(void) {
    Mesa mesa;
    tp_listad *historico;
    ResultadoFluxo status = JOGO_OK;
    int codigoSaida = 0;
    int consultar;

    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);   // Faz o terminal do Windows entender UTF-8 (os naipes)
        SetConsoleCP(CP_UTF8);
    #endif

    srand((unsigned) time(NULL));   // Aleatoriedade apenas no embaralhamento, no inicio de cada mao

    historico = inicializarHistorico();
    if (historico == NULL) {
        puts("Sem memoria para inicializar o VAR.");
        return 1;
    }
    historico->partidaEncerrada = 0;

    if (!configurarMesa(&mesa)) {
        puts("Nao foi possivel cadastrar a mesa (ou entrada encerrada).");
        destruirHistorico(historico);
        return 1;
    }
    mostrarPlacar(&mesa);

    // Joga rodadas ate uma dupla chegar em PONTOS_PARA_VENCER
    while (mesa.pontosDupla1 < PONTOS_PARA_VENCER &&
           mesa.pontosDupla2 < PONTOS_PARA_VENCER) {
        status = jogarMao(&mesa, historico);
        if (status != JOGO_OK) break;
    }

    if (status == JOGO_SAIR) {
        puts("\nPartida encerrada por escolha do jogador.\n");
    } else if (status == JOGO_ERRO) {
        puts("\nPartida interrompida por erro ou fim da entrada.");
        codigoSaida = 1;
    } else {
        int vencedora = mesa.pontosDupla1 >= PONTOS_PARA_VENCER ? 1 : 2;
        finalizarPartidaHistorico(historico);
        printf("\n========== FIM DA PARTIDA ==========\n");
        printf("A Dupla %d venceu o Truco no Bar do Pato!\n", vencedora);
        mostrarPlacar(&mesa);
    }

    printf("O VAR registrou %d rodada(s) nesta partida.\n", historico->tamanho);
    if (!verificarHistoricoVazio(historico)) {
        if (lerNumero("Deseja consultar o VAR? (1=Sim, 2=Nao): ",
                      1, 2, &consultar) && consultar == 1) {
            consultarVar(historico, mesa.jogadores);
        }
    }
    historico = destruirHistorico(historico);
    return codigoSaida;
}
