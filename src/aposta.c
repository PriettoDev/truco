#include "aposta.h"

//retorna o proximo valor permitido no truco paulista
static int proximo_valor(int valor) {
    if (valor == 1) return 3;
    if (valor == 3) return 6;
    if (valor == 6) return 12;
    return 0;
}

//inicia cada mao valendo um ponto
void aposta_iniciar(Aposta *a) {
    if (a == 0) return;
    a->valor = 1;
    a->pedido = APOSTA_SEM_PEDIDO;
    a->dupla_pediu = APOSTA_DUPLA_INVALIDA;
    a->dupla_responde = APOSTA_DUPLA_INVALIDA;
    a->ultima_dupla_pediu = APOSTA_DUPLA_INVALIDA;
    a->encerrada = 0;
    a->dupla_vencedora = APOSTA_DUPLA_INVALIDA;
    a->pontos_ganhos = 0;
}

//verifica se uma dupla precisa responder a um pedido
int aposta_pendente(const Aposta *a) {
    return a != 0 && a->pedido != APOSTA_SEM_PEDIDO;
}

//faz um pedido ou aumenta um pedido recebido
int aposta_pedir(Aposta *a, int dupla) {
    int novo_valor;
    if (a == 0 || a->encerrada || (dupla != 0 && dupla != 1)) return 0;

    if (aposta_pendente(a)) {
        //somente quem recebeu o pedido pode aumentar
        if (dupla != a->dupla_responde) return 0;
        novo_valor = proximo_valor(a->pedido);
        if (novo_valor == 0) return 0;
        //a contraproposta confirma o valor anterior
        a->valor = a->pedido;
    } else {
        //a mesma dupla nao pode pedir duas vezes seguidas
        if (dupla == a->ultima_dupla_pediu) return 0;
        novo_valor = proximo_valor(a->valor);
        if (novo_valor == 0) return 0;
    }

    a->pedido = novo_valor;
    a->dupla_pediu = dupla;
    a->dupla_responde = 1 - dupla;
    a->ultima_dupla_pediu = dupla;
    return 1;
}

//confirma o valor solicitado pela dupla adversaria
int aposta_aceitar(Aposta *a, int dupla) {
    if (a == 0 || a->encerrada || !aposta_pendente(a) || dupla != a->dupla_responde) return 0;
    a->valor = a->pedido;
    a->pedido = APOSTA_SEM_PEDIDO;
    a->dupla_responde = APOSTA_DUPLA_INVALIDA;
    return 1;
}

//encerra a mao e concede o valor confirmado a quem pediu
int aposta_correr(Aposta *a, int dupla) {
    if (a == 0 || a->encerrada || !aposta_pendente(a) || dupla != a->dupla_responde) return 0;
    a->dupla_vencedora = a->dupla_pediu;
    a->pontos_ganhos = a->valor;
    a->encerrada = 1;
    a->pedido = APOSTA_SEM_PEDIDO;
    a->dupla_responde = APOSTA_DUPLA_INVALIDA;
    return 1;
}
