#include <stdio.h>
#include "professor.h"

#define FRASES_POR_EVENTO 3

// associa cada evento a um vetor de tres frases
// proxima guarda a posicao da frase que sera exibida
typedef struct {
    EventoProfessor evento;
    char frases[FRASES_POR_EVENTO][150];
    int proxima;
} FalaProfessor;

// vetor de estruturas com as reacoes do personagem ficticio
static FalaProfessor falas[] = {
    {PROF_INICIO, {
        "Vamos prestar atencao nas regras!",
        "Conhecimento nao cai do ceu!",
        "Hoje a abstracao vai ser na mesa de truco!"
    }, 0},
    {PROF_INVALIDA, {
        "Eu devo estar falando javanes, russo ou hebraico!",
        "Voces prestaram atencao no que eu falei?",
        "Vamos ler as opcoes com calma!"
    }, 0},
    {PROF_PEDIDO, {
        "O Dev-C++ vai ficar abstrato!",
        "Quero ver a logica desse pedido!",
        "Agora vamos testar essa estrategia!"
    }, 0},
    {PROF_ACEITE, {
        "Parece que alguem estudou!",
        "Vamos ver se essa logica funciona!",
        "Coragem e bom, mas estudar tambem!"
    }, 0},
    {PROF_CORRIDA, {
        "Voces acham que as coisas vao cair do ceu?",
        "Nao estudou e agora quer correr?",
        "Vamos revisar essa estrategia para a proxima mao!"
    }, 0},
    {PROF_MAO_VENCIDA, {
        "Finalmente alguem prestou atencao!",
        "Agora essa abstracao fez sentido!",
        "Muito bem, mas ainda nao acabou!"
    }, 0},
    {PROF_EMPATE, {
        "Vamos precisar de mais uma aula!",
        "Essa logica ainda nao resolveu o problema!",
        "Empate tambem faz parte do algoritmo!"
    }, 0},
    {PROF_FIM, {
        "Espero que na prova seja assim tambem!",
        "Parabens! Mas continuem estudando!",
        "A aula acabou, a abstracao continua!"
    }, 0}
};

const char *professor_frase(EventoProfessor evento) {
    // calcula a quantidade de registros do vetor
    int quantidade = sizeof(falas) / sizeof(falas[0]);

    // busca sequencial pelo acontecimento recebido
    for (int i = 0; i < quantidade; i++) {
        if (falas[i].evento == evento) {
            int posicao = falas[i].proxima;
            // alterna as frases e volta ao inicio depois da ultima
            falas[i].proxima = (posicao + 1) % FRASES_POR_EVENTO;
            return falas[i].frases[posicao];
        }
    }
    // nao exibe comentario para um evento desconhecido
    return "";
}

void professor_comentar(EventoProfessor evento) {
    // recebe o endereco da frase sem copiar o texto
    const char *frase = professor_frase(evento);
    if (frase[0] != '\0') {
        printf("\n[professor do bar] %s\n", frase);
    }
}
