#ifndef ARVORE_H
#define ARVORE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "filale.h"

#define MAX 27

int indprefixa;

typedef int tp_item;

typedef struct tp_no {
    struct tp_no *esq;
    tp_item info;
    struct tp_no *dir;
} tp_no;

typedef tp_no *tp_arvore;

tp_arvore inicializa_arvore() {
    return NULL;
}

int arvore_vazia(tp_arvore raiz) {
    if (raiz == NULL) return 1;
    else return 0;
}

tp_no *aloca_no() {
    tp_no *no;
    no = (tp_no *)malloc(sizeof(tp_no));
    return no;
}

int insere_no(tp_arvore *raiz, tp_item e) {
    tp_no *pai = NULL, *novo, *p = *raiz;
    novo = aloca_no();
    if (!novo) return 0;
    novo->info = e;
    novo->esq = NULL;
    novo->dir = NULL;
    while (p != NULL) {
        pai = p;
        if (e < p->info)
            p = p->esq;
        else
            p = p->dir;
    }

    if (pai != NULL) {
        if (e < pai->info)
            pai->esq = novo;
        else
            pai->dir = novo;
    }
    else
        *raiz = novo;

    return 1;
}

void pre_ordem(tp_no *p) {
    if (p != NULL) {
        printf("%c", p->info);
        pre_ordem(p->esq);
        pre_ordem(p->dir);
    }
}

void em_ordem(tp_no *p) {
    if (p != NULL) {
        em_ordem(p->esq);
        printf("%c", p->info);
        em_ordem(p->dir);
    }
}

void pos_ordem(tp_no *p) {
    if (p != NULL) {
        pos_ordem(p->esq);
        pos_ordem(p->dir);
        printf("%c", p->info);
    }
}

int remove_no(tp_arvore *raiz, tp_item e) {
    tp_no *p = *raiz, *ant = NULL, *sub, *pai, *filho;
    while (p != NULL && p->info != e) {
        ant = p;
        if (e < p->info)
            p = p->esq;
        else
            p = p->dir;
    }
    if (p == NULL) /* nao encontrou */
        return 0;
    /* no tem no maximo um filho */
    if (p->esq == NULL)
        sub = p->dir;
    else if (p->dir == NULL)
        sub = p->esq;
    else {
        /* no tem dois filhos */
        pai = p; sub = p->dir; filho = sub->esq;
        while (filho != NULL) {
            pai = sub; sub = filho; filho = sub->esq;
        }
        /* neste ponto, sub e o sucessor em ordem de p */
        if (pai != p) {
            pai->esq = sub->dir;
            sub->dir = p->dir;
        }
        sub->esq = p->esq;
    }
    /* insere sub na posicao ocupada por p */
    if (ant == NULL)
        *raiz = sub; /* p era raiz */
    else if (p == ant->esq)
        ant->esq = sub;
    else
        ant->dir = sub;
    free(p);
    return 1;
}

void destroi_no(tp_no *no) {
    if (no == NULL) return;
    destroi_no(no->esq);
    destroi_no(no->dir);
    free(no);
}

void destroi_arvore(tp_arvore *raiz) {
    if (*raiz == NULL) return;
    destroi_no(*raiz);
    *raiz = NULL;
}

void imprime_bfs(tp_arvore *raiz) {
    if (!arvore_vazia(*raiz)) {
        tp_fila *fila = inicializaFila();
        tp_item_fila item;
        insereFila(fila, *raiz);

        while (!filaVazia(fila)) {
            removeFila(fila, &item);
            tp_no *no = (tp_no *) item;
            printf("%c", no->info);

            if (no->esq != NULL) insereFila(fila, no->esq);
            if (no->dir != NULL) insereFila(fila, no->dir);
        }
        fila = destroiFila(fila);
    }
}

int busca_infixa(char *infixa, int esq, int dir, tp_item info) {
    int i;
    for (i = esq; i <= dir; i++) {
        if (infixa[i] == info)
            return i;
    }
    return -1;   /* so retorna -1 depois de percorrer tudo */
}

tp_arvore monta_arvore(char prefixa[], char infixa[], int esq, int dir) {
    if (esq > dir)
        return NULL;

    tp_no *no = aloca_no();
    no->info = prefixa[indprefixa];
    no->dir = no->esq = NULL;
    indprefixa++;

    if (esq == dir)
        return no;

    int indinfixa = busca_infixa(infixa, esq, dir, no->info);
    no->esq = monta_arvore(prefixa, infixa, esq, indinfixa - 1);
    no->dir = monta_arvore(prefixa, infixa, indinfixa + 1, dir);
    return no;
}

void espelha_arvore(tp_no *p) {
    tp_no *aux;

    if (p != NULL) {
        aux = p->esq;
        p->esq = p->dir;
        p->dir = aux;

        espelha_arvore(p->esq);
        espelha_arvore(p->dir);
    }
}

#endif