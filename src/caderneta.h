#ifndef CADERNETA_H
#define CADERNETA_H

#include <stddef.h>
#include "arvore_avl.h"

#define CADERNETA_NOME_MAX AVL_CHAVE_MAX

typedef struct {
    ArvAVL *raiz;
    size_t quantidade;
    char arquivo[260];
} Caderneta;

typedef struct {
    const char *nome;
    unsigned long long partidas_jogadas;
    unsigned long long maos_ganhas;
    unsigned long long trucos_pedidos;
    unsigned long long vezes_correu;
} CadernetaDelta;

// Carrega a caderneta. Se o arquivo ainda nao existir, comeca vazia
int caderneta_inicializar(Caderneta *caderneta, const char *arquivo);
void caderneta_destruir(Caderneta *caderneta);

// Busca na AVL por nome: O(log n). Retorna o nome armazenado ou NULL
const FreguesAVL *caderneta_buscar(const Caderneta *caderneta, const char *nome);

// Insere um fregues na AVL e salva imediatamente
// Retorna 1 se inseriu, 0 se ja existia, -1 em caso de erro
int caderneta_adicionar(Caderneta *caderneta, const char *nome);

int caderneta_aplicar(Caderneta *caderneta, const CadernetaDelta deltas[], size_t quantidade);
int caderneta_salvar(const Caderneta *caderneta);
void caderneta_listar(const Caderneta *caderneta);

#endif