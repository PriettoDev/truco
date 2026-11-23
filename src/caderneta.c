#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "caderneta.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define sync_file(fd) _commit(fd)
#define file_fd(arquivo) _fileno(arquivo)
#else
#include <unistd.h>
#define sync_file(fd) _commit(fd)
#define file_fd(arquivo) _fileno(arquivo)
#endif

#define CADERNETA_LIMITE 10000u
#define CABECALHO "# CADERNETA_TRUCO_BAR_DO_PATO\n"

typedef struct {
    FILE *arquivo;
    int ok;
} ContextoGravacao;

static int nome_valido(const char *nome) {
    size_t tamanho;

    if (nome == NULL) return 0;
    tamanho = strlen(nome);
    if (tamanho == 0 || tamanho >= CADERNETA_NOME_MAX) return 0;

    return strpbrk(nome, "\t\r\n|") == NULL;
}

static void gravar_no(const FreguesAVL *fregues, void *arg) {
    ContextoGravacao *contexto = arg;

    if (contexto->ok &&
        fprintf(contexto->arquivo, "%-29s | %16llu | %11llu | %14llu | %16llu\n",
            fregues->nome,
            fregues->partidas_jogadas,
            fregues->maos_ganhas,
            fregues->trucos_pedidos,
            fregues->vezes_correu) < 0){
        contexto->ok = 0;
    }
}

int caderneta_salvar(const Caderneta *caderneta) {
    char temporario[300];
    FILE *arquivo;
    ContextoGravacao contexto;
    int ok;

    if (caderneta == NULL || caderneta->raiz == NULL 
        || caderneta->arquivo[0] == '\0' || caderneta->quantidade > CADERNETA_LIMITE) {
        return 0;
    }

    if (snprintf(temporario, sizeof temporario, "%s.tmp", caderneta->arquivo) >= (int)sizeof temporario) {
        return 0;
    }

    arquivo = fopen(temporario, "w");
    if (arquivo == NULL) return 0;

    contexto.arquivo = arquivo;
    contexto.ok = fputs(CABECALHO, arquivo) >= 0;

    if (contexto.ok &&
        fprintf(arquivo, "# %-29s | %16s | %11s | %14s | %16s\n",
            "Nome",
            "Partidas jogadas",
            "Maos ganhas",
            "Trucos pedidos",
            "Vezes que correu") < 0) {
        contexto.ok = 0;
    }

    arvore_avl_em_ordem( caderneta->raiz, gravar_no, &contexto);
    ok = contexto.ok;

    if (ok && fflush(arquivo) != 0) ok = 0;

    if (ok && sync_file(file_fd(arquivo)) != 0) ok = 0;

    if (fclose(arquivo) != 0) ok = 0;

    if (ok) {
#ifdef _WIN32
        ok = MoveFileExA(temporario, caderneta->arquivo, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        ok = rename(temporario, caderneta->arquivo) == 0;
#endif
    }

    if (!ok) remove(temporario);
    return ok;
}

static int parse_u64(char *texto, unsigned long long *resultado) {
    char *fim;
    unsigned long long valor;

    if (texto == NULL || *texto == '\0' || *texto == '-') return 0;
    errno = 0;

    valor = strtoull(texto, &fim, 10);
    if (errno != 0 || fim == texto || *fim != '\0') return 0;
    *resultado = valor;
    return 1;
}

static char *aparar_espacos(char *texto) {
    char *fim;

    while (*texto == ' ' || *texto == '\t') {
        texto++;
    }
    fim = texto + strlen(texto);
    while (fim > texto && (fim[-1] == ' ' || fim[-1] == '\t')) {
        *--fim = '\0';
    }

    return texto;
}

static int carregar_linha(Caderneta *caderneta, char *linha, char anterior[CADERNETA_NOME_MAX], size_t *quantidade, int legado) {
    char *campos[5];
    char *separador;
    char separadorCampo;
    size_t tamanho = strlen(linha);
    FreguesAVL *fregues;

    if (tamanho == 0 || linha[tamanho - 1] != '\n') return 0;

    linha[--tamanho] = '\0';

    if (tamanho > 0 && linha[tamanho - 1] == '\r') {
        linha[--tamanho] = '\0';
    }

    // Ignora linhas de identificação e o cabeçalho das colunas
    if (linha[0] == '#') return 1;
    campos[0] = linha;

    if (legado) {
        campos[0] = aparar_espacos(campos[0]);

        if (!nome_valido(campos[0])) return 0;
    } else {
        separadorCampo = strchr(linha, '|') != NULL ? '|' : '\t';

        for (int i = 1; i < 5; i++) {
            separador = strchr(campos[i - 1], separadorCampo);
            if (separador == NULL) return 0;

            *separador = '\0';
            campos[i] = separador + 1;
        }

        if (strchr(campos[4], separadorCampo) != NULL) return 0;

        for (int i = 0; i < 5; i++) {
            campos[i] = aparar_espacos(campos[i]);
        }

        if (!nome_valido(campos[0])) return 0;
    }

    if ((*quantidade > 0 && strcmp(anterior, campos[0]) >= 0) || *quantidade >= CADERNETA_LIMITE) {
        return 0;
    }

    if (arvore_avl_inserir(caderneta->raiz, campos[0]) != 1) return 0;

    fregues = arvore_avl_buscar_mutavel(caderneta->raiz, campos[0]);
    if (fregues == NULL) return 0;

    if (!legado &&
        (!parse_u64(campos[1], &fregues->partidas_jogadas) ||
         !parse_u64(campos[2], &fregues->maos_ganhas) ||
         !parse_u64(campos[3], &fregues->trucos_pedidos) ||
         !parse_u64(campos[4], &fregues->vezes_correu))) {
        return 0;
    }

    strcpy(anterior, campos[0]);
    (*quantidade)++;

    return 1;
}

int caderneta_inicializar(Caderneta *caderneta, const char *arquivo_nome) {
    FILE *arquivo;
    char linha[256];
    char anterior[CADERNETA_NOME_MAX] = "";
    size_t quantidade = 0;
    int legado;

    if (caderneta == NULL || arquivo_nome == NULL || strlen(arquivo_nome) >= sizeof caderneta->arquivo) {
        return 0;
    }

    caderneta->raiz = arvore_avl_criar();
    caderneta->quantidade = 0;
    caderneta->arquivo[0] = '\0';

    if (caderneta->raiz == NULL) return 0;
    strcpy(caderneta->arquivo, arquivo_nome);
    arquivo = fopen(arquivo_nome, "r");

    if (arquivo == NULL) return errno == ENOENT;
    if (fgets(linha, sizeof linha, arquivo) == NULL) goto erro;
    legado = strcmp(linha, CABECALHO) != 0;

    if (legado &&
        !carregar_linha( caderneta, linha, anterior, &quantidade, 1)) {
        goto erro;
    }

    while (fgets(linha, sizeof linha, arquivo) != NULL) {
        if (!carregar_linha(caderneta, linha, anterior, &quantidade, legado)) {
            goto erro;
        }
    }

    if (ferror(arquivo) || fclose(arquivo) != 0) {
        caderneta_destruir(caderneta);
        return 0;
    }
    caderneta->quantidade = quantidade;
    return 1;

erro:
    fclose(arquivo);
    caderneta_destruir(caderneta);
    return 0;
}

void caderneta_destruir(Caderneta *caderneta) {
    if (caderneta == NULL) return;

    arvore_avl_destruir(caderneta->raiz);
    caderneta->raiz = NULL;
    caderneta->quantidade = 0;
    caderneta->arquivo[0] = '\0';
}

const FreguesAVL *caderneta_buscar(const Caderneta *caderneta, const char *nome) {
    if (caderneta == NULL || caderneta->raiz == NULL) return NULL;
    return arvore_avl_buscar(caderneta->raiz, nome);
}

int caderneta_adicionar(Caderneta *caderneta, const char *nome) {
    int resultado;

    if (caderneta == NULL || caderneta->raiz == NULL || !nome_valido(nome) || caderneta->quantidade >= CADERNETA_LIMITE) return -1;
    resultado = arvore_avl_inserir(caderneta->raiz, nome);

    if (resultado != 1) return resultado;
    caderneta->quantidade++;

    if (!caderneta_salvar(caderneta)) {
        (void)arvore_avl_remover(caderneta->raiz, nome);
        caderneta->quantidade--;
        return -1;
    }

    return 1;
}

static int somar(unsigned long long *valor, unsigned long long incremento) {
    if (ULLONG_MAX - *valor < incremento) return 0;
    *valor += incremento;
    return 1;
}

int caderneta_aplicar(Caderneta *caderneta, const CadernetaDelta deltas[], size_t quantidade) {
    FreguesAVL *alvos[4];
    FreguesAVL antigos[4];

    if (caderneta == NULL || deltas == NULL || quantidade > 4) return 0;

    for (size_t i = 0; i < quantidade; i++) {
        if (deltas[i].nome == NULL) return 0;

        alvos[i] = arvore_avl_buscar_mutavel(caderneta->raiz, deltas[i].nome);

        if (alvos[i] == NULL) return 0;
        antigos[i] = *alvos[i];
    }

    for (size_t i = 0; i < quantidade; i++) {
        if (!somar(&alvos[i]->partidas_jogadas, deltas[i].partidas_jogadas) ||
            !somar(&alvos[i]->maos_ganhas, deltas[i].maos_ganhas) ||
            !somar(&alvos[i]->trucos_pedidos, deltas[i].trucos_pedidos) ||
            !somar(&alvos[i]->vezes_correu, deltas[i].vezes_correu)) {
            for (size_t j = 0; j <= i; j++) {
                *alvos[j] = antigos[j];
            }

            return 0;
        }
    }

    if (!caderneta_salvar(caderneta)) {
        for (size_t i = 0; i < quantidade; i++) {
            *alvos[i] = antigos[i];
        }
        return 0;
    }
    return 1;
}

static void imprimir_fregues(const FreguesAVL *fregues, void *contexto) {
    (void)contexto;

    printf("%-29s | partidas: %llu | maos ganhas: %llu" " | trucos pedidos: %llu | correu: %llu\n",
        fregues->nome,
        fregues->partidas_jogadas,
        fregues->maos_ganhas,
        fregues->trucos_pedidos,
        fregues->vezes_correu
    );
}

void caderneta_listar(const Caderneta *caderneta) {
    if (caderneta == NULL || caderneta->raiz == NULL) return;

    printf("\n--- CADERNETA DO BAR: %zu fregues(es) ---\n", caderneta->quantidade);

    if (caderneta->quantidade == 0) puts("Ainda nao ha fregueses cadastrados.");

    arvore_avl_em_ordem(caderneta->raiz, imprimir_fregues, NULL);
}