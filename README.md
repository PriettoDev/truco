# Truco no Bar do Pato

Projeto desenvolvido para a disciplina de *Algoritmos e Estruturas de Dados*, do curso de Engenharia da Computação da Universidade SENAI CIMATEC.

O projeto tem como objetivo implementar, em linguagem C, uma versão do *Truco Paulista*, aplicando os conceitos de estruturas de dados, modularização e manipulação de TADs estudados durante a disciplina.

## Estado atual

O projeto está na **Unidade 2**: uma partida completa de Truco pode ser jogada no terminal por 4 jogadores, do cadastro até a vitória de uma dupla aos 12 pontos, com o histórico de todas as rodadas disponível para consulta (VAR).

| Versão | Conteúdo |
|--------|----------|
| `v1.0` | Unidade 1: cartas, baralho, jogadores, duplas e organização da mesa |
| `v2.0` | Unidade 2: rodadas, mãos, apostas (truco), pontuação e VAR |

## Funcionalidades implementadas

### Unidade 1

- Criação das 40 cartas do baralho;
- Embaralhamento e retirada de cartas pelo topo;
- Cadastro de 4 jogadores;
- Organização dos jogadores em 2 duplas alternadas;
- Distribuição de 3 cartas para cada jogador;
- Revelação da vira e identificação da manilha;
- Organização da ordem dos jogadores;
- Rotação do jogador "mão".

### Unidade 2

- Partida completa entre duas duplas, encerrada quando uma delas chega a 12 pontos;
- Rodadas com uma carta por jogador e apuração da carta vencedora, considerando a manilha e o desempate por naipe;
- Mãos de até 3 rodadas, com tratamento de rodadas empatadas;
- Pedido de truco e aumento da aposta (3, 6 e 12), com resposta obrigatória da dupla adversária: aceitar, aumentar ou correr;
- Bloqueio de pedidos consecutivos da mesma dupla;
- Placar atualizado ao fim de cada mão;
- Todas as jogadas são escolhidas pelos jogadores (não há jogadas automáticas);
- Leitura de entradas com validação de opções e de nomes repetidos;
- Exibição dos naipes em UTF-8 também no terminal do Windows;
- **VAR**: histórico de todas as rodadas da partida (vira, manilha, cartas jogadas, eventos de aposta e dupla vencedora), que pode ser percorrido para frente e para trás ao final do jogo.

## Estruturas de dados utilizadas

| Estrutura | Onde é usada | Para quê |
|-----------|--------------|----------|
| Vetor com controle de topo | `Baralho` | Retirar as cartas sempre pelo topo do baralho embaralhado |
| Fila circular (alocação estática) | `Mesa` / `Ordem` | Manter a ordem de jogo e girar o jogador "mão" a cada nova mão |
| Lista duplamente encadeada (alocação dinâmica) | `historico` (VAR) | Guardar cada rodada da partida e navegar entre elas nos dois sentidos |
| `struct` e `enum` | Todos os módulos | Representar cartas, jogadores, rodadas, mãos e apostas |

## Estrutura do projeto

```text
truco/
├── src/           código-fonte do jogo
├── test/          testes de cada módulo
├── docs/          documentação e registros da equipe
├── README.md
└── LICENSE
```

O código foi dividido em módulos de acordo com suas responsabilidades:

- `carta.c` / `carta.h`: representação das cartas, valores, naipes, manilha e força de cada carta;
- `baralho.c` / `baralho.h`: criação, embaralhamento e retirada das cartas;
- `jogador.c` / `jogador.h`: dados dos jogadores, dupla, assento na mesa e cartas da mão;
- `mesa.c` / `mesa.h`: organização da mesa, ordem de jogo (fila circular) e placar;
- `rodada.c` / `rodada.h`: jogadas de uma rodada e apuração da carta vencedora;
- `aposta.c` / `aposta.h`: pedidos de truco, aumentos, aceite e corrida;
- `mao.c` / `mao.h`: controle de uma mão (até 3 rodadas), aposta e pontos ganhos;
- `historico.c` / `historico.h`: VAR, o histórico da partida em lista duplamente encadeada;
- `main.c`: entradas do usuário, sequência da partida, pontuação e gravação do VAR.

## Compilação e execução

É necessário ter um compilador C instalado (por exemplo, GCC/MinGW ou o TDM-GCC do Dev-C++).

Na raiz do repositório:

```bash
gcc -Wall -Wextra src/*.c -o truco
```

Depois, execute:

```bash
./truco        # Linux / macOS
truco.exe      # Windows
```

Durante a execução, o programa pede os nomes dos quatro jogadores, forma as duplas e conduz a partida mão a mão. Em cada vez de jogar, o jogador escolhe uma carta ou pede truco. Ao fim da partida, é possível consultar o VAR.

> No Dev-C++, crie um projeto em C e adicione todos os arquivos `.c` e `.h` da pasta `src/`.

## Testes

Cada módulo possui um teste na pasta `test/`. Para compilar um teste, inclua o arquivo de teste e os módulos de `src/` (exceto o `main.c`):

```bash
gcc -Wall -Wextra test/testeMao.c src/carta.c src/baralho.c src/jogador.c src/mesa.c src/rodada.c src/aposta.c src/mao.c src/historico.c -o testeMao
```

Troque `testeMao.c` por `testeCarta.c`, `testeBaralho.c`, `testeJogador.c`, `testeMesa.c` ou `testeRodada.c` para testar os demais módulos.

## Tecnologias

- Linguagem C
- Estruturas de Dados
- Git e GitHub

## Licença

Este projeto está sob a licença MIT. Consulte o arquivo `LICENSE`.