#ifndef FILALE_H
#define FILALE_H

#include <stdio.h>
#include <stdlib.h>

/* A fila guarda ponteiros genericos (ex.: tp_no* da arvore) */
typedef void *tp_item_fila;

typedef struct tp_no_fila {
	tp_item_fila info;
	struct tp_no_fila *prox;
} tp_no_fila;

typedef struct {
	tp_no_fila *ini, *fim;
} tp_fila;

/* declaracao de funcoes */
tp_no_fila *alocaNoFila();
int removeFila(tp_fila *fila, tp_item_fila *e);
int insereFila(tp_fila *fila, tp_item_fila e);
int filaVazia(tp_fila *fila);
tp_fila *destroiFila(tp_fila *fila);

/* funcoes basicas da biblioteca */
tp_fila *inicializaFila() {
	tp_fila *fila = (tp_fila *) malloc(sizeof(tp_fila));
	if (!fila) return NULL;
	fila->ini = fila->fim = NULL;
	return fila;
}

tp_no_fila *alocaNoFila() {
	return (tp_no_fila *) malloc(sizeof(tp_no_fila));
}

int filaVazia(tp_fila *fila) {
	if (fila->ini == NULL) return 1;
	return 0;
}

int insereFila(tp_fila *fila, tp_item_fila e) {
	tp_no_fila *novo = alocaNoFila();

	if (!novo) return 0;

	novo->info = e;
	novo->prox = NULL;

	if (filaVazia(fila))      /* primeiro elemento */
		fila->ini = novo;
	else
		fila->fim->prox = novo;

	fila->fim = novo;
	return 1;
}

int removeFila(tp_fila *fila, tp_item_fila *e) {
	tp_no_fila *aux;

	if (filaVazia(fila)) return 0;

	*e = fila->ini->info;
	aux = fila->ini;

	if (fila->ini == fila->fim)   /* unico no da fila */
		fila->ini = fila->fim = NULL;
	else
		fila->ini = fila->ini->prox;

	free(aux);
	return 1;
}

tp_fila *destroiFila(tp_fila *fila) {
	tp_item_fila e;

	while (!filaVazia(fila))
		removeFila(fila, &e);

	free(fila);
	return NULL;
}

#endif