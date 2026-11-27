#ifndef PROFESSOR_H
#define PROFESSOR_H

// identifica os acontecimentos que podem gerar comentarios
// os nomes possuem prefixo para evitar conflitos com outros modulos
typedef enum {
    PROF_INICIO,
    PROF_INVALIDA,
    PROF_PEDIDO,
    PROF_ACEITE,
    PROF_CORRIDA,
    PROF_MAO_VENCIDA,
    PROF_EMPATE,
    PROF_FIM
} EventoProfessor;

// retorna uma frase do evento ou uma string vazia se nao existir
const char *professor_frase(EventoProfessor evento);

// apresenta a frase no terminal
void professor_comentar(EventoProfessor evento);

#endif
