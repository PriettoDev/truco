#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/professor.h"

int main(void) {
    // verifica se cada evento possui tres frases diferentes e ciclicas
    for (int i = PROF_INICIO; i <= PROF_FIM; i++) {
        const char *a = professor_frase((EventoProfessor)i);
        const char *b = professor_frase((EventoProfessor)i);
        const char *c = professor_frase((EventoProfessor)i);
        assert(a[0] && b[0] && c[0]);
        assert(strcmp(a, b) != 0);
        assert(strcmp(a, c) != 0);
        assert(strcmp(b, c) != 0);
        assert(strcmp(a, professor_frase((EventoProfessor)i)) == 0);
    }
    // verifica se eventos invalidos nao retornam frases
    assert(professor_frase((EventoProfessor)-1)[0] == '\0');
    assert(professor_frase((EventoProfessor)99)[0] == '\0');
    puts("testes do professor aprovados");
    return 0;
}
