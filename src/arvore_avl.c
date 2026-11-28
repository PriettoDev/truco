#include "arvore_avl.h"

#include <stdlib.h>
#include <string.h>

static int maior(int a, int b) {
    if (a > b) return a;
    else return b;
}

static int altura(ArvAVL no) {
    if (no == NULL) return -1;
    else return no->altura;
}

static int fator_balanceamento(ArvAVL no) {
    if (no == NULL) return 0;
    else return altura(no->esq) - altura(no->dir);
}

static void atualizar_altura(ArvAVL no) {
    if (no != NULL) no->altura = maior(altura(no->esq), altura(no->dir)) + 1;
}

static void RotacaoLL(ArvAVL *raiz) {
    ArvAVL nova_raiz = (*raiz)->esq;
    (*raiz)->esq = nova_raiz->dir;
    nova_raiz->dir = *raiz;

    atualizar_altura(*raiz);
    atualizar_altura(nova_raiz);
    *raiz = nova_raiz;
}

static void RotacaoRR(ArvAVL *raiz) {
    ArvAVL nova_raiz = (*raiz)->dir;
    (*raiz)->dir = nova_raiz->esq;
    nova_raiz->esq = *raiz;
    
    atualizar_altura(*raiz);
    atualizar_altura(nova_raiz);
    *raiz = nova_raiz;
}

static void RotacaoLR(ArvAVL *raiz) {
    RotacaoRR(&(*raiz)->esq);
    RotacaoLL(raiz);
}

static void RotacaoRL(ArvAVL *raiz) {
    RotacaoLL(&(*raiz)->dir);
    RotacaoRR(raiz);
}

static void rebalancear(ArvAVL *raiz) {
    int fb;
    if (raiz == NULL || *raiz == NULL) return;
    atualizar_altura(*raiz);
    fb = fator_balanceamento(*raiz);
    
    if (fb > 1) {
        if (fator_balanceamento((*raiz)->esq) < 0) RotacaoLR(raiz);
        else RotacaoLL(raiz);
    } else if (fb < -1) {
        if (fator_balanceamento((*raiz)->dir) > 0) RotacaoRL(raiz);
        else RotacaoRR(raiz);
    }
}

ArvAVL *arvore_avl_criar(void) {
    ArvAVL *raiz = malloc(sizeof *raiz);
    if (raiz != NULL) *raiz = NULL;
    return raiz;
}

static void destruir_nos(ArvAVL no) {
    if (no == NULL) return;
    destruir_nos(no->esq);
    destruir_nos(no->dir);
    free(no);
}

void arvore_avl_destruir(ArvAVL *raiz) {
    if (raiz == NULL) return;
    destruir_nos(*raiz);
    free(raiz);
}

static int inserir_recursivo(ArvAVL *raiz, const char *chave) {
    int comparacao;
    ArvAVL novo;
    int resultado;

    if (*raiz == NULL) {
        novo = malloc(sizeof *novo);
        if (novo == NULL) return -1;

        memset(&novo->fregues, 0, sizeof novo->fregues);
        strcpy(novo->fregues.nome, chave);

        novo->altura = 0;
        novo->esq = NULL;
        novo->dir = NULL;
        *raiz = novo;
        return 1;
    }
    comparacao = strcmp(chave, (*raiz)->fregues.nome);

    if (comparacao == 0) return 0;
    if (comparacao < 0) {
        resultado = inserir_recursivo(&(*raiz)->esq, chave);

        if (resultado == 1) rebalancear(raiz);
        return resultado;
    }

    resultado = inserir_recursivo(&(*raiz)->dir, chave);
    if (resultado == 1) rebalancear(raiz);
    return resultado;
}

int arvore_avl_inserir(ArvAVL *raiz, const char *chave) {
    size_t tamanho;

    if (raiz == NULL || chave == NULL) return -1;
    tamanho = strlen(chave);
    if (tamanho == 0 || tamanho >= AVL_CHAVE_MAX) return -1;

    return inserir_recursivo(raiz, chave);
}

const FreguesAVL *arvore_avl_buscar(const ArvAVL *raiz, const char *chave) {
    ArvAVL atual;

    if (raiz == NULL || chave == NULL) return NULL;
    
    atual = *raiz;

    while (atual != NULL) {
        int comparacao = strcmp(chave, atual->fregues.nome);
        if (comparacao == 0) return &atual->fregues;

        atual = comparacao < 0 ? atual->esq : atual->dir;
    }
    return NULL;
}

FreguesAVL *arvore_avl_buscar_mutavel(ArvAVL *raiz, const char *chave) {
    return (FreguesAVL *)arvore_avl_buscar((const ArvAVL *)raiz, chave);
}

static ArvAVL menor_no(ArvAVL no) {
    while (no != NULL && no->esq != NULL){
        no = no->esq;
    }
    return no;
}

static int remover_recursivo(ArvAVL *raiz, const char *chave) {
    int comparacao;
    ArvAVL removido;

    if (*raiz == NULL) return 0;
    
    comparacao = strcmp(chave, (*raiz)->fregues.nome);
    if (comparacao < 0) {
        int resultado = remover_recursivo(&(*raiz)->esq, chave);
        if (resultado == 1) rebalancear(raiz);
        return resultado;
    }

    if (comparacao > 0) {
        int resultado = remover_recursivo(&(*raiz)->dir, chave);
        if (resultado == 1) rebalancear(raiz);
        return resultado;
    }

    if ((*raiz)->esq == NULL || (*raiz)->dir == NULL) {
        removido = *raiz;
        if (removido->esq != NULL) {
            *raiz = removido->esq;
        } else{
            *raiz = removido->dir;
        }

        free(removido);
        return 1;
    }
    //Bloco anônimo, serve para caso separar instruções. Nesse caso, é usadeo quando o nó tem dois filhos
    {
        ArvAVL sucessor = menor_no((*raiz)->dir);
        (*raiz)->fregues = sucessor->fregues;
        (void)remover_recursivo(&(*raiz)->dir, sucessor->fregues.nome);
        
        rebalancear(raiz);
        return 1;
    }
}

int arvore_avl_remover(ArvAVL *raiz, const char *chave) {
    if (raiz == NULL || chave == NULL) return 0;
    return remover_recursivo(raiz, chave);
}

static void em_ordem_recursivo(const ArvAVL *raiz, void (*visitar)(const FreguesAVL *, void *), void *contexto) {
    if (raiz == NULL || *raiz == NULL || visitar == NULL) return;

    em_ordem_recursivo(&(*raiz)->esq, visitar, contexto);
    visitar(&(*raiz)->fregues, contexto);
    em_ordem_recursivo(&(*raiz)->dir, visitar, contexto);
}

void arvore_avl_em_ordem(const ArvAVL *raiz, void (*visitar)(const FreguesAVL *, void *), void *contexto) {
    em_ordem_recursivo(raiz, visitar, contexto);
}