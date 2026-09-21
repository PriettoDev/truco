# Truco no Bar do Pato

Projeto desenvolvido para a disciplina de *Algoritmos e Estruturas de Dados*, do curso de Engenharia da Computação da Universidade SENAI CIMATEC.

O projeto tem como objetivo implementar, em linguagem C, uma versão do *Truco Paulista*, aplicando os conceitos de estruturas de dados, modularização e manipulação de TADs estudados durante a disciplina.

## Funcionalidades implementadas

Nesta primeira etapa, o projeto contempla:

- Criação das 40 cartas do baralho;
- Embaralhamento e retirada de cartas pelo topo;
- Cadastro de 4 jogadores;
- Organização dos jogadores em 2 duplas alternadas;
- Distribuição de 3 cartas para cada jogador;
- Revelação da vira e identificação da manilha;
- Organização da ordem dos jogadores;
- Rotação do jogador "mão".

## Estrutura do projeto

O código foi dividido em módulos de acordo com suas responsabilidades:

- carta.c / carta.h - representação das cartas, valores, naipes e manilha;
- baralho.c / baralho.h - criação, embaralhamento e retirada das cartas;
- jogador.c / jogador.h - dados dos jogadores, duplas e cartas da mão;
- mesa.c / mesa.h - organização da mesa e ordem dos jogadores;
- TesteGeral.c - integração e teste dos módulos desenvolvidos.

## Tecnologias

- Linguagem C
- Estruturas de Dados
- Git e GitHub

## Execução

Durante a execução, serão solicitados os nomes dos quatro jogadores e apresentados o baralho, a distribuição das cartas e a ordem da mesa.
