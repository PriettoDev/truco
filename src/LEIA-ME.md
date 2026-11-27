# professor do bar — fiscal de algoritmos

base: truco-develop (2).zip. este pacote contém apenas os quatro arquivos da contribuição, não o repositório completo.

## github
crie feature/professor-do-bar a partir de develop. envie professor.c e professor.h para src/ e testeProfessor.c para test/. compare main.c com a versão atual antes de substituir; se houver novos commits, incorpore somente o include e as chamadas professor_comentar. não envie o zip inteiro. não inclua o sistema de conquistas anterior.

## funcionamento
24 frases em 8 eventos. um enum identifica o evento, uma struct guarda três frases e a próxima posição. a busca sequencial encontra o evento e alterna as frases circularmente, sem usar rand, para não interferir na sequência de números aleatórios do baralho. custo O(n) para n tipos de evento; memória fixa. as frases são uma homenagem fictícia e não alteram as regras. professor_frase retorna um ponteiro para texto estático, útil para futura interface gráfica. não há integração gráfica neste pacote.

## testes (na raiz do repositório completo)
```
gcc -std=c11 -Wall -Wextra -Wpedantic src/*.c -o truco.exe
gcc -std=c11 -Wall -Wextra -Wpedantic test/testeProfessor.c src/professor.c -o testeProfessor.exe
```
execute os programas gerados. o teste verifica os oito eventos, alternância circular e eventos inválidos. a versão terminal foi compilada e testada com entrada simulada. não foi realizado merge com as branches dos colegas.
