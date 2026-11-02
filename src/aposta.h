#ifndef APOSTA_H
#define APOSTA_H

#define APOSTA_SEM_PEDIDO 0
#define APOSTA_DUPLA_INVALIDA -1

typedef struct {
    int valor;                 //valor confirmado da mao
    int pedido;                //valor solicitado, ou zero
    int dupla_pediu;           //dupla 0 ou 1 que fez o pedido
    int dupla_responde;        //dupla 0 ou 1 que deve responder
    int ultima_dupla_pediu;    //impede pedidos consecutivos da mesma dupla
    int encerrada;             //indica encerramento por corrida
    int dupla_vencedora;       //dupla que ganhou por corrida
    int pontos_ganhos;         //valor anterior ao pedido recusado
} Aposta;

void aposta_iniciar(Aposta *a);
int aposta_pedir(Aposta *a, int dupla);
int aposta_aceitar(Aposta *a, int dupla);
int aposta_correr(Aposta *a, int dupla);
int aposta_pendente(const Aposta *a);

#endif
