/*
 * teste_partida.c - teste automatico de partida_gui.c, sem janela.
 *
 * Joga 200 partidas escolhendo cartas ao acaso (semente fixa, para o
 * resultado ser sempre o mesmo) e confere que:
 *  - toda partida termina com uma dupla em 12;
 *  - nenhuma funcao de src/ falhou;
 *  - nenhuma carta se repete entre as 12 distribuidas e a vira, e as
 *    jogadas mais as que sobraram sao exatamente as distribuidas;
 *  - quem abre a mao gira 0, 1, 2, 3; o vencedor abre a rodada seguinte e,
 *    no empate, abre quem abriu; a ordem e anti-horaria;
 *  - a mao so aparece na propria vez e as travas de clique funcionam;
 *  - o destaque da apuracao segue a regra do rodada_apurar;
 *  - truco pedido e respondido ao acaso: os valores seguem a sequencia do
 *    aposta.c, a mesma dupla nunca pede duas vezes seguidas, o botao so
 *    fica ativo quando o modulo aceitaria, quem responde e o proximo depois
 *    de quem fez o ultimo pedido, a corrida da o valor confirmado a dupla
 *    que pediu, quem estava na vez volta a jogar depois do aceite, nenhum
 *    placar passa de 12 e cada pedido/resposta gera um evento;
 *  - ninguem ve a mao fora da propria vez ou da propria resposta.
 * Tambem roda casos montados de empate, de parceiros com cartas iguais e
 * de truco (aceito, corrido, seis corrido, doze aceito, passar de 12...).
 *
 * Rodar: make -C gui teste (sai com codigo 1 se algo falhar).
 */
#include <stdio.h>
#include <stdlib.h>
#include "partida_gui.h"

static int erros = 0;
#define CONFERE(cond, ...) do { if (!(cond)) { erros++; if (erros <= 20) { printf("ERRO: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

static int mesma(Carta a, Carta b) { return a.naipe == b.naipe && a.valor == b.valor; }

/* Estatisticas */
static int totMaos, totRodadas, totEmpates, totParceirosIguais, totMaosEmpatadas, totTravas;
static int totPedidos, totAceites, totAumentos, totCorridas, totDozes, totBotaoRecusado;

/* Sequencia de valores esperada: a do aposta.c hoje (1, 3, 6, 12). E de
   proposito uma tabela do teste, e nao uma chamada ao modulo: se o time
   mudar o aposta.c, o teste avisa, e esta tabela e atualizada junto. */
static int proximo_esperado(int valor) {
    if (valor == 1) return 3;
    if (valor == 3) return 6;
    if (valor == 6) return 12;
    return 0;
}
static int valor_da_sequencia(int v) { return v == 1 || v == 3 || v == 6 || v == 12; }

/* Modelo do teste para a mao atual: o que a negociacao deveria estar. */
static int valorT, pedidoT, ultimaDuplaT, voltaPara, corridaNaMao;
static unsigned contadorT;

static void conferir_evento(const PartidaGui *p, TipoEvento tipo, int assento, int valor, const char *oque) {
    contadorT++;
    CONFERE(p->contadorEventos == contadorT, "%s: contador %u, esperado %u", oque, p->contadorEventos, contadorT);
    CONFERE(p->ultimoEvento.tipo == tipo && p->ultimoEvento.assento == assento && p->ultimoEvento.valor == valor,
            "%s: evento %d/%d/%d, esperado %d/%d/%d", oque, (int)p->ultimoEvento.tipo, p->ultimoEvento.assento,
            p->ultimoEvento.valor, (int)tipo, assento, valor);
}

/* Avanca ate sair do estado de espera, passando tempo pequeno. */
static void esperar(PartidaGui *p, float t) { partida_atualizar(p, t); }

/* Confere as regras de travamento e privacidade no estado atual. */
static void conferir_estado(PartidaGui *p) {
    EstadoPartida e = p->estado;
    int visivel = partida_mao_visivel(p);
    if (e == PARTIDA_VEZ_DO_JOGADOR || e == PARTIDA_CARTA_VOANDO)
        CONFERE(visivel == p->assentoDaVez, "mao deveria aparecer no estado %d", (int)e);
    else if (e == PARTIDA_RESPOSTA)
        CONFERE(visivel == p->assentoResponde, "na resposta so quem responde ve a mao (visivel %d)", visivel);
    else
        CONFERE(visivel == -1, "mao visivel no estado %d", (int)e);
    /* Em INICIO_MAO a Mao ainda nao foi iniciada (zerada na partida nova). */
    if (e != PARTIDA_INICIO_MAO)
        CONFERE(valor_da_sequencia(p->mao.aposta.valor), "valor %d fora da sequencia", p->mao.aposta.valor);
    CONFERE(p->mao.aposta.pedido == 0 || valor_da_sequencia(p->mao.aposta.pedido),
            "pedido %d fora da sequencia", p->mao.aposta.pedido);
    CONFERE(partida_pontos(p, 0) <= 12 && partida_pontos(p, 1) <= 12, "placar passou de 12: %d x %d",
            partida_pontos(p, 0), partida_pontos(p, 1));
    if (e == PARTIDA_CARTA_VOANDO || e == PARTIDA_APURANDO || e == PARTIDA_ANUNCIO_PEDIDO) {
        int antes = p->estado;
        CONFERE(partida_confirmar(p) == 0 && (int)p->estado == antes, "clique aceito no estado %d", antes);
        totTravas++;
    }
}

/* Confere o destaque logo que a apuracao comeca. */
static void conferir_apuracao(PartidaGui *p) {
    const Rodada *r = &p->mao.rodadas[p->mao.rodada_atual - 1];
    Valor m = p->mao.manilha;
    totRodadas++;
    CONFERE(p->resultadoRodada == r->dupla_vencedora, "resultado diferente do modulo");
    /* parceiros com cartas iguais (mesma forca) nesta rodada */
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 4; j++)
            if (r->jogadas[i].dupla == r->jogadas[j].dupla &&
                carta_comparar(r->jogadas[i].carta, r->jogadas[j].carta, m) == 0) totParceirosIguais++;

    if (r->dupla_vencedora == EMPATE) {
        totEmpates++;
        CONFERE(p->destaque[0] >= 0 && p->destaque[1] >= 0, "empate sem dois destaques");
        if (p->destaque[0] >= 0 && p->destaque[1] >= 0) {
            CartaNaMesa a = p->naMesa[p->destaque[0]], b = p->naMesa[p->destaque[1]];
            CONFERE(a.assento % 2 == 0 && b.assento % 2 == 1, "destaques do empate nao sao um de cada dupla");
            CONFERE(carta_comparar(a.carta, b.carta, m) == 0, "destaques do empate com forcas diferentes");
            for (int i = 0; i < p->qtdNaMesa; i++)
                CONFERE(carta_forca(p->naMesa[i].carta, m) <= carta_forca(a.carta, m), "carta mais forte que o destaque do empate");
        }
    } else {
        CONFERE(p->destaque[0] >= 0 && p->destaque[1] == -1, "vitoria com destaque errado");
        if (p->destaque[0] >= 0) {
            CartaNaMesa v = p->naMesa[p->destaque[0]];
            CONFERE(v.assento == r->jogador_vencedor, "destaque nao e do jogador vencedor");
            CONFERE(v.assento % 2 == r->dupla_vencedora, "destaque de outra dupla");
            for (int i = 0; i < p->qtdNaMesa; i++)
                if (p->naMesa[i].assento % 2 != v.assento % 2)
                    CONFERE(carta_comparar(v.carta, p->naMesa[i].carta, m) > 0, "vencedora nao vence o adversario");
        }
    }
}

/* Uma partida inteira com cartas ao acaso. Devolve 1 se terminou. */
static int jogar_partida(PartidaGui *p, int numero) {
    Carta dadas[13];       /* 12 distribuidas + vira */
    Carta jogadas[12];
    int qtdJogadas = 0, numMao = 0, abreEsperado = 0, abreRodada = -1;
    int passos = 0;

    partida_iniciar(p);
    while (p->estado != PARTIDA_FIM_PARTIDA) {
        if (++passos > 100000) { CONFERE(0, "partida %d nao terminou", numero); return 0; }
        conferir_estado(p);

        switch (p->estado) {
        case PARTIDA_INICIO_MAO:
            esperar(p, 0.016f);
            /* acabou de distribuir: confere cartas repetidas e quem abre */
            numMao++;
            totMaos++;
            for (int s = 0; s < 4; s++) {
                CONFERE(p->mesa.jogadores[s].qtdCartas == 3, "jogador %d sem 3 cartas", s);
                for (int c = 0; c < 3; c++) dadas[s * 3 + c] = p->mesa.jogadores[s].mao[c];
            }
            dadas[12] = p->mesa.vira;
            for (int i = 0; i < 13; i++)
                for (int j = i + 1; j < 13; j++)
                    CONFERE(!mesma(dadas[i], dadas[j]), "carta repetida na mao %d da partida %d", numMao, numero);
            CONFERE(p->mao.jogador_abre == abreEsperado, "mao %d abriu com %d, esperado %d",
                    numMao, p->mao.jogador_abre, abreEsperado);
            CONFERE(p->assentoDaVez == abreEsperado, "primeiro da vez errado");
            CONFERE(p->mao.manilha == manilha_da_vira(p->mesa.vira), "manilha errada");
            abreRodada = abreEsperado;
            abreEsperado = (abreEsperado + 1) % 4;
            qtdJogadas = 0;
            valorT = 1; pedidoT = 0; ultimaDuplaT = -1; voltaPara = -1; corridaNaMao = 0;
            CONFERE(p->mao.aposta.valor == 1, "mao comecou valendo %d", p->mao.aposta.valor);
            break;
        case PARTIDA_COBERTURA:
            /* clique cedo demais (clique duplo) tem que ser ignorado */
            CONFERE(partida_confirmar(p) == 0 && p->estado == PARTIDA_COBERTURA, "cobertura aceitou clique antes de 0,3 s");
            esperar(p, 0.31f);
            CONFERE(partida_confirmar(p) == 1 && p->estado == PARTIDA_VEZ_DO_JOGADOR, "cobertura nao abriu");
            break;
        case PARTIDA_VEZ_DO_JOGADOR: {
            Jogador *j = &p->mesa.jogadores[p->assentoDaVez];
            int qtd = j->qtdCartas;
            int pos = rand() % qtd;
            Carta escolhida = j->mao[pos];
            if (p->qtdNaMesa == 0)
                CONFERE(p->assentoDaVez == abreRodada, "rodada aberta por %d, esperado %d", p->assentoDaVez, abreRodada);
            else
                CONFERE(p->assentoDaVez == (p->naMesa[p->qtdNaMesa - 1].assento + 1) % 4, "ordem anti-horaria quebrada");
            if (voltaPara >= 0) {
                /* acabou uma negociacao aceita: a vez e de quem estava jogando */
                CONFERE(p->assentoDaVez == voltaPara, "depois do aceite a vez foi para %d, esperado %d",
                        p->assentoDaVez, voltaPara);
                CONFERE(p->mao.aposta.valor == valorT, "mao vale %d depois do aceite, esperado %d",
                        p->mao.aposta.valor, valorT);
                voltaPara = -1;
            }
            {   /* botao de pedir: ativo exatamente quando a regra permite */
                int s = p->assentoDaVez, prox = -1;
                int pode = partida_pode_pedir(p, &prox);
                int esperado = (s % 2 != ultimaDuplaT) && proximo_esperado(valorT) != 0;
                CONFERE(pode == esperado, "botao de pedir %d, esperado %d (assento %d, valor %d, ultima dupla %d)",
                        pode, esperado, s, valorT, ultimaDuplaT);
                if (!pode) {
                    totBotaoRecusado++;
                    CONFERE(partida_pedir_truco(p) == 0, "pedido aceito com o botao desativado");
                }
                if (pode && esperado) {
                    CONFERE(prox == proximo_esperado(valorT), "botao pediria %d, esperado %d", prox, proximo_esperado(valorT));
                    if (rand() % 4 == 0) {
                        CONFERE(partida_pedir_truco(p) == 0, "pedido aceito antes da trava");
                        esperar(p, 0.31f);
                        CONFERE(partida_pedir_truco(p) == 1, "pedido valido recusado");
                        CONFERE(p->estado == PARTIDA_ANUNCIO_PEDIDO, "pedido nao foi para o anuncio");
                        conferir_evento(p, EVENTO_PEDIU, s, prox, "pedido");
                        CONFERE(p->assentoPediu == s && p->assentoResponde == (s + 1) % 4,
                                "pediu %d responde %d, esperado %d e %d", p->assentoPediu, p->assentoResponde, s, (s + 1) % 4);
                        pedidoT = prox;
                        ultimaDuplaT = s % 2;
                        voltaPara = s;
                        totPedidos++;
                        break;
                    }
                }
            }
            CONFERE(partida_jogar_carta(p, pos) == 0, "carta saiu antes da trava");
            esperar(p, 0.31f);
            CONFERE(partida_jogar_carta(p, qtd) == 0, "posicao invalida aceita");
            CONFERE(partida_jogar_carta(p, pos) == 1, "carta valida recusada");
            CONFERE(j->qtdCartas == qtd - 1, "carta nao saiu da mao");
            CONFERE(mesma(p->naMesa[p->qtdNaMesa - 1].carta, escolhida), "carta na mesa diferente da escolhida");
            jogadas[qtdJogadas++] = escolhida;
            break;
        }
        case PARTIDA_CARTA_VOANDO:
            esperar(p, 0.2f);
            CONFERE(p->estado == PARTIDA_CARTA_VOANDO, "voo acabou antes de 0,35 s");
            esperar(p, 0.2f);
            if (p->estado == PARTIDA_APURANDO) conferir_apuracao(p);
            break;
        case PARTIDA_APURANDO: {
            int resultado = p->resultadoRodada, abreAntes = abreRodada;
            int vencedor = p->mao.rodadas[p->mao.rodada_atual - 1].jogador_vencedor;
            esperar(p, 1.0f);
            CONFERE(p->estado == PARTIDA_APURANDO, "apuracao acabou antes de 1,5 s");
            esperar(p, 0.6f);
            if (p->estado == PARTIDA_COBERTURA) {
                /* vencedor abre; no empate abre quem abriu */
                abreRodada = (resultado == EMPATE) ? abreAntes : vencedor;
                CONFERE(p->qtdNaMesa == 0, "mesa nao foi limpa");
            }
            break;
        }
        case PARTIDA_ANUNCIO_PEDIDO:
            CONFERE(partida_assento_visto(p) == p->assentoPediu, "anuncio sem a camera de quem pediu");
            esperar(p, 1.0f);
            CONFERE(p->estado == PARTIDA_ANUNCIO_PEDIDO, "anuncio acabou antes de 1,2 s");
            esperar(p, 0.3f);
            CONFERE(p->estado == PARTIDA_COBERTURA_RESPOSTA, "anuncio nao passou para a cobertura da resposta");
            break;
        case PARTIDA_COBERTURA_RESPOSTA:
            CONFERE(partida_assento_visto(p) == p->assentoResponde, "cobertura sem a camera de quem responde");
            CONFERE(partida_confirmar(p) == 0, "cobertura da resposta aceitou clique antes de 0,3 s");
            esperar(p, 0.31f);
            CONFERE(partida_confirmar(p) == 1 && p->estado == PARTIDA_RESPOSTA, "cobertura da resposta nao abriu");
            break;
        case PARTIDA_RESPOSTA: {
            int r = p->assentoResponde, pediu = p->assentoPediu, prox = -1, acao;
            int podeMais = partida_pode_pedir(p, &prox);
            CONFERE(r == (pediu + 1) % 4, "responde %d depois do pedido de %d", r, pediu);
            CONFERE(r % 2 != pediu % 2, "quem responde (%d) e da dupla de quem pediu (%d)", r, pediu);
            CONFERE(p->mao.aposta.pedido == pedidoT, "pedido pendente %d, esperado %d", p->mao.aposta.pedido, pedidoT);
            CONFERE(podeMais == (proximo_esperado(pedidoT) != 0), "pedir mais %d com pedido %d", podeMais, pedidoT);
            if (podeMais) CONFERE(prox == proximo_esperado(pedidoT), "pedir mais pediria %d", prox);
            CONFERE(partida_responder(p, RESPOSTA_ACEITAR) == 0, "resposta aceita antes da trava");
            esperar(p, 0.31f);

            acao = rand() % 10;   /* 0-5 aceita, 6-7 corre, 8-9 pede mais (se puder) */
            if (acao >= 8 && !podeMais) {
                CONFERE(partida_responder(p, RESPOSTA_AUMENTAR) == 0, "aumento alem do maximo aceito");
                acao = 0;
            }
            if (acao <= 5) {
                CONFERE(partida_responder(p, RESPOSTA_ACEITAR) == 1, "aceite recusado");
                valorT = pedidoT;
                pedidoT = 0;
                if (valorT == 12) totDozes++;
                conferir_evento(p, EVENTO_ACEITOU, r, valorT, "aceite");
                CONFERE(p->estado == PARTIDA_COBERTURA && p->assentoDaVez == voltaPara,
                        "depois do aceite: estado %d, vez %d, esperado cobertura de %d",
                        (int)p->estado, p->assentoDaVez, voltaPara);
                totAceites++;
            } else if (acao <= 7) {
                int d = pediu % 2, antes = partida_pontos(p, d), outra = partida_pontos(p, 1 - d);
                int esperado = antes + valorT > 12 ? 12 : antes + valorT;
                CONFERE(partida_responder(p, RESPOSTA_CORRER) == 1, "corrida recusada");
                conferir_evento(p, EVENTO_CORREU, r, valorT, "corrida");
                CONFERE(p->estado == PARTIDA_FIM_MAO, "corrida nao acabou a mao");
                CONFERE(p->duplaVencedoraMao == d, "corrida deu a mao a dupla %d, esperado %d", p->duplaVencedoraMao, d);
                CONFERE(p->pontosDaMao == valorT, "corrida valeu %d, esperado %d (valor confirmado)", p->pontosDaMao, valorT);
                CONFERE(partida_pontos(p, d) == esperado && partida_pontos(p, 1 - d) == outra,
                        "placar depois da corrida %d x %d", partida_pontos(p, 0), partida_pontos(p, 1));
                CONFERE(p->assentoCorreu == r, "assentoCorreu %d, esperado %d", p->assentoCorreu, r);
                corridaNaMao = 1;
                pedidoT = 0;
                totCorridas++;
            } else {
                CONFERE(partida_responder(p, RESPOSTA_AUMENTAR) == 1, "aumento valido recusado");
                valorT = pedidoT;          /* o aumento confirma o pedido anterior */
                pedidoT = prox;
                ultimaDuplaT = r % 2;
                conferir_evento(p, EVENTO_AUMENTOU, r, prox, "aumento");
                CONFERE(p->estado == PARTIDA_ANUNCIO_PEDIDO, "aumento nao foi para o anuncio");
                CONFERE(p->mao.aposta.valor == valorT, "aumento nao confirmou o valor anterior");
                totAumentos++;
            }
            break;
        }
        case PARTIDA_FIM_MAO: {
            /* jogadas + as que sobraram na mao = distribuidas */
            int total = qtdJogadas;
            Carta todas[12];
            for (int i = 0; i < qtdJogadas; i++) todas[i] = jogadas[i];
            for (int s = 0; s < 4; s++)
                for (int c = 0; c < p->mesa.jogadores[s].qtdCartas; c++) todas[total++] = p->mesa.jogadores[s].mao[c];
            CONFERE(total == 12, "faltam cartas no fim da mao (%d)", total);
            for (int i = 0; i < 12 && total == 12; i++) {
                int achou = 0;
                for (int k = 0; k < 12; k++) achou += mesma(dadas[i], todas[k]);
                CONFERE(achou == 1, "carta distribuida aparece %d vez(es) no fim da mao", achou);
            }
            if (corridaNaMao) {
                CONFERE(p->assentoCorreu >= 0, "mao corrida sem quem correu");
            } else if (p->duplaVencedoraMao == MAO_EMPATE) {
                totMaosEmpatadas++;
                CONFERE(p->pontosDaMao == 0, "mao empatada pontuou");
                CONFERE(p->assentoCorreu == -1, "mao jogada ate o fim com assentoCorreu");
            } else {
                CONFERE(p->pontosDaMao == valorT, "mao valeu %d, esperado %d (valor confirmado)", p->pontosDaMao, valorT);
                CONFERE(p->assentoCorreu == -1, "mao jogada ate o fim com assentoCorreu");
            }
            CONFERE(p->mesa.numeroDaRodada == numMao + 1, "numero da mao nao avancou");
            CONFERE(partida_confirmar(p) == 0, "fim da mao aceitou clique antes de 0,3 s");
            esperar(p, 0.31f);
            CONFERE(partida_confirmar(p) == 1, "fim da mao nao avancou");
            break;
        }
        default:
            esperar(p, 0.016f);
            break;
        }
    }
    {
        int a = partida_pontos(p, 0), b = partida_pontos(p, 1), v = p->duplaVencedoraPartida;
        CONFERE((a == 12) != (b == 12), "placar final %d x %d", a, b);
        CONFERE(v >= 0 && partida_pontos(p, v) == 12, "vencedora %d com placar %d x %d", v, a, b);
    }
    return 1;
}

/* ---- casos montados: troca as cartas logo depois da distribuicao ---- */

/* Joga a rodada inteira com a 1a carta de cada um e devolve o estado da apuracao. */
static void jogar_rodada_montada(PartidaGui *p, Carta vira, const Carta porAssento[4], const char *nome,
                                 int resultadoEsperado, int dest0, int dest1) {
    partida_iniciar(p);
    partida_atualizar(p, 0.016f);              /* INICIO_MAO -> COBERTURA, assento 0 abre */
    p->mesa.vira = vira;
    p->manilha = manilha_da_vira(vira);
    p->mao.manilha = p->manilha;
    for (int s = 0; s < 4; s++) p->mesa.jogadores[s].mao[0] = porAssento[s];

    for (int k = 0; k < 4; k++) {
        partida_atualizar(p, 0.31f);
        partida_confirmar(p);
        partida_atualizar(p, 0.31f);
        partida_jogar_carta(p, 0);
        partida_atualizar(p, 0.4f);
    }
    CONFERE(p->estado == PARTIDA_APURANDO, "%s: nao chegou na apuracao", nome);
    CONFERE(p->resultadoRodada == resultadoEsperado, "%s: resultado %d, esperado %d", nome, p->resultadoRodada, resultadoEsperado);
    {
        int a0 = p->destaque[0] >= 0 ? p->naMesa[p->destaque[0]].assento : -1;
        int a1 = p->destaque[1] >= 0 ? p->naMesa[p->destaque[1]].assento : -1;
        CONFERE(a0 == dest0 && a1 == dest1, "%s: destaques nos assentos %d e %d, esperado %d e %d",
                nome, a0, a1, dest0, dest1);
        printf("  %-46s resultado %2d | destaque assentos %2d e %2d\n", nome, p->resultadoRodada, a0, a1);
    }
    CONFERE(p->falhas == 0, "%s: %d falhas", nome, p->falhas);
}

/* ---- casos montados de truco: sequencias fixas de pedidos e respostas ---- */

/* Do INICIO_MAO ate a vez do assento 0, depois da trava. */
static void ate_vez_do_assento0(PartidaGui *p, int pontos0, int pontos1) {
    partida_iniciar(p);
    p->mesa.pontosDupla1 = pontos0;
    p->mesa.pontosDupla2 = pontos1;
    partida_atualizar(p, 0.016f);
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
}

/* Passa o anuncio e a cobertura e responde. Devolve o que o modulo disse. */
static int responder(PartidaGui *p, RespostaTruco r) {
    partida_atualizar(p, 1.3f);
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    return partida_responder(p, r);
}

static void caso_truco(const char *nome, int ok) {
    CONFERE(ok, "caso de truco: %s", nome);
    printf("  %-58s %s\n", nome, ok ? "ok" : "FALHOU");
}

static void casos_de_truco(PartidaGui *p) {
    char nome[16];
    int prox, ok;

    printf("Casos montados de truco:\n");

    ate_vez_do_assento0(p, 0, 0);
    ok = partida_pedir_truco(p) && responder(p, RESPOSTA_ACEITAR);
    ok = ok && p->mao.aposta.valor == 3 && p->estado == PARTIDA_COBERTURA && p->assentoDaVez == 0 &&
         p->ultimoEvento.tipo == EVENTO_ACEITOU && p->ultimoEvento.assento == 1;
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    ok = ok && p->estado == PARTIDA_VEZ_DO_JOGADOR && !partida_pode_pedir(p, NULL) && !partida_pedir_truco(p);
    caso_truco("truco aceito: vale 3, volta ao 0, dupla 0 nao pede de novo", ok);

    ate_vez_do_assento0(p, 0, 0);
    ok = partida_pedir_truco(p) && responder(p, RESPOSTA_CORRER);
    caso_truco("truco corrido: dupla 0 leva 1", ok && p->estado == PARTIDA_FIM_MAO &&
               p->duplaVencedoraMao == 0 && p->pontosDaMao == 1 && partida_pontos(p, 0) == 1 &&
               partida_pontos(p, 1) == 0 && p->assentoCorreu == 1);

    ate_vez_do_assento0(p, 0, 0);
    ok = partida_pedir_truco(p) && responder(p, RESPOSTA_AUMENTAR);
    ok = ok && p->assentoPediu == 1 && p->assentoResponde == 2 && p->mao.aposta.valor == 3 && p->mao.aposta.pedido == 6;
    ok = ok && responder(p, RESPOSTA_CORRER);
    caso_truco("seis em cima do truco, Tonhao corre: dupla 1 leva 3", ok &&
               p->duplaVencedoraMao == 1 && p->pontosDaMao == 3 && partida_pontos(p, 1) == 3 && p->assentoCorreu == 2);

    ate_vez_do_assento0(p, 0, 0);
    ok = partida_pedir_truco(p) && responder(p, RESPOSTA_AUMENTAR) && responder(p, RESPOSTA_AUMENTAR);
    ok = ok && p->assentoPediu == 2 && p->assentoResponde == 3 && p->mao.aposta.pedido == 12;
    /* na resposta ao doze nao ha "pedir mais" */
    partida_atualizar(p, 1.3f);
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    ok = ok && p->estado == PARTIDA_RESPOSTA && !partida_pode_pedir(p, NULL) &&
         !partida_responder(p, RESPOSTA_AUMENTAR) && partida_responder(p, RESPOSTA_ACEITAR);
    ok = ok && p->mao.aposta.valor == 12 && p->assentoDaVez == 0;
    caso_truco("truco, seis, doze aceito: vale 12 e volta ao 0", ok);

    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    ok = p->estado == PARTIDA_VEZ_DO_JOGADOR && !partida_pode_pedir(p, &prox) && prox == 0 && !partida_pedir_truco(p);
    partida_jogar_carta(p, 0);                       /* o assento 1 (outra dupla) tambem nao passa de 12 */
    partida_atualizar(p, 0.4f);
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    ok = ok && p->assentoDaVez == 1 && !partida_pode_pedir(p, NULL) && !partida_pedir_truco(p);
    caso_truco("depois do doze ninguem pede mais (nem a outra dupla)", ok);

    ate_vez_do_assento0(p, 11, 0);
    ok = partida_pedir_truco(p) && responder(p, RESPOSTA_AUMENTAR) && responder(p, RESPOSTA_AUMENTAR) &&
         responder(p, RESPOSTA_CORRER);
    caso_truco("11 x 0, doze corrido pela dupla 1: placar para em 12", ok &&
               p->pontosDaMao == 6 && partida_pontos(p, 0) == 12);

    ate_vez_do_assento0(p, 0, 0);
    ok = partida_pedir_truco(p) && responder(p, RESPOSTA_ACEITAR);
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    partida_jogar_carta(p, 0);
    partida_atualizar(p, 0.4f);
    partida_atualizar(p, 0.31f);
    partida_confirmar(p);
    partida_atualizar(p, 0.31f);
    ok = ok && p->assentoDaVez == 1 && partida_pode_pedir(p, &prox) && prox == 6 && partida_pedir_truco(p) &&
         p->assentoResponde == 2;
    caso_truco("depois do truco aceito, a outra dupla pede seis", ok);

    ok = 1;
    {
        const int valores[] = { 3, 6, 9, 12, 15 };
        const char *nomes[] = { "TRUCO", "SEIS", "NOVE", "DOZE", "15" };
        for (int i = 0; i < 5; i++) {
            partida_nome_do_valor(valores[i], nome, (int)sizeof nome);
            for (int c = 0; nome[c] || nomes[i][c]; c++) ok = ok && nome[c] == nomes[i][c];
        }
    }
    caso_truco("rotulos: 3 TRUCO, 6 SEIS, 9 NOVE, 12 DOZE, 15 -> \"15\"", ok);
    CONFERE(p->falhas == 0, "casos de truco: %d falhas", p->falhas);
}

int main(void) {
    static PartidaGui p;   /* static: nunca copiar, e a struct e grande */
    int falhasTotais = 0, terminaram = 0, vitorias[2] = { 0, 0 };
    Carta vira = { OUROS, QUATRO };   /* manilha = CINCO */

    srand(12345);
    for (int i = 0; i < 200; i++) {
        contadorT = p.contadorEventos;   /* o contador nunca volta a zero */
        terminaram += jogar_partida(&p, i);
        falhasTotais += p.falhas;
        if (p.duplaVencedoraPartida == 0 || p.duplaVencedoraPartida == 1) vitorias[p.duplaVencedoraPartida]++;
    }

    printf("Casos montados (vira 4 de ouros, manilha 5):\n");
    {   /* empate entre duplas: 3 x 3, os parceiros com 4 */
        Carta c[4] = { { ESPADAS, TRES }, { COPAS, TRES }, { PAUS, QUATRO }, { COPAS, QUATRO } };
        jogar_rodada_montada(&p, vira, c, "empate entre duplas (3 x 3)", EMPATE, 0, 1);
    }
    {   /* parceiros com cartas iguais: 3 e 3 contra 2 e 2 -> dupla 0 vence, fica o 1o 3 */
        Carta c[4] = { { ESPADAS, TRES }, { COPAS, DOIS }, { PAUS, TRES }, { OUROS, DOIS } };
        jogar_rodada_montada(&p, vira, c, "parceiros iguais (3 e 3 contra 2 e 2)", 0, 0, -1);
    }
    {   /* parceiros iguais dos dois lados e empate: destaca o 1o de cada dupla */
        Carta c[4] = { { ESPADAS, REI }, { COPAS, REI }, { PAUS, REI }, { OUROS, REI } };
        jogar_rodada_montada(&p, vira, c, "quatro reis (empate, 1o de cada dupla)", EMPATE, 0, 1);
    }
    {   /* manilha desempata pelo naipe: 5 de paus vence 5 de copas */
        Carta c[4] = { { COPAS, CINCO }, { PAUS, CINCO }, { ESPADAS, TRES }, { OUROS, TRES } };
        jogar_rodada_montada(&p, vira, c, "manilhas (5 de paus vence 5 de copas)", 1, 1, -1);
    }

    casos_de_truco(&p);

    printf("\nRESUMO\n");
    printf("  partidas jogadas ate o fim : %d de 200\n", terminaram);
    printf("  vitorias por dupla interna : dupla 0 (Seu Bigode/Tonhão) %d | dupla 1 (Dona Marlene/Kiko) %d\n",
           vitorias[0], vitorias[1]);
    printf("  maos / rodadas             : %d / %d\n", totMaos, totRodadas);
    printf("  rodadas empatadas          : %d\n", totEmpates);
    printf("  parceiros com cartas iguais: %d\n", totParceirosIguais);
    printf("  maos empatadas (0 pontos)  : %d\n", totMaosEmpatadas);
    printf("  cliques bloqueados conferidos: %d\n", totTravas);
    printf("  truco: pedidos %d | aceites %d | aumentos %d | corridas %d | dozes aceitos %d\n",
           totPedidos, totAceites, totAumentos, totCorridas, totDozes);
    printf("  botao de pedir desativado conferido: %d vezes\n", totBotaoRecusado);
    printf("  falhas de funcoes de src/  : %d\n", falhasTotais);
    printf("  erros do teste             : %d\n", erros);
    CONFERE(totEmpates > 0 && totParceirosIguais > 0, "faltou caso natural de empate ou parceiros iguais");
    CONFERE(totAceites > 0 && totAumentos > 0 && totCorridas > 0 && totDozes > 0 && totBotaoRecusado > 0,
            "faltou algum caso natural de truco");
    printf("%s\n", (erros == 0 && falhasTotais == 0 && terminaram == 200) ? "TUDO CERTO" : "FALHOU");
    return erros != 0 || falhasTotais != 0;
}
