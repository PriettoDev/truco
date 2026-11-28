#ifndef ARVORE_AVL_H
#define ARVORE_AVL_H

#define AVL_CHAVE_MAX 30

typedef struct AVLNo *ArvAVL;

typedef struct {
    char nome[AVL_CHAVE_MAX];
    unsigned long long partidas_jogadas;
    unsigned long long maos_ganhas;
    unsigned long long trucos_pedidos;
    unsigned long long vezes_correu;
} FreguesAVL;

struct AVLNo {
    FreguesAVL fregues;
    int altura;
    ArvAVL esq;
    ArvAVL dir;
};

// Retornos da insercao: 1 inserido, 0 duplicado, -1 invalido/sem memoria.
ArvAVL *arvore_avl_criar(void);
void arvore_avl_destruir(ArvAVL *raiz);
int arvore_avl_inserir(ArvAVL *raiz, const char *chave);
int arvore_avl_remover(ArvAVL *raiz, const char *chave);
const FreguesAVL *arvore_avl_buscar(const ArvAVL *raiz, const char *chave);
FreguesAVL *arvore_avl_buscar_mutavel(ArvAVL *raiz, const char *chave);
void arvore_avl_em_ordem( const ArvAVL *raiz, void (*visitar)(const FreguesAVL *, void *), void *contexto);


#endif