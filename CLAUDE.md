# Truco no Bar do Pato — interface gráfica (raylib)

## Contexto
Trabalho da disciplina AED (SENAI CIMATEC), em C. Entrega em 29/11/2026.
A apresentação será provavelmente em Windows; o desenvolvimento é em macOS.

## Regras invioláveis
- NUNCA alterar os módulos de regra (carta, rodada, mao, aposta, jogador, mesa,
  historico, baralho) nem o main.c do terminal. A interface só lê o estado e
  chama funções existentes. Se faltar uma função, PARE e me avise.
- A versão de terminal deve continuar compilando: é a entrega oficial do edital.
- A branch base é develop (a main está desatualizada).
- Os módulos ficam em src/. A interface fica em gui/ e inclui os .h de ../src/.
  Recursos gráficos e de áudio ficam em assets/.
- Executáveis são gerados em output/ (já ignorado pelo .gitignore).
- ATENÇÃO à numeração das duplas: rodada, mao e aposta usam 0 e 1;
  Jogador.dupla e historico usam 1 e 2. Dentro da interface, use sempre
  assento % 2 para a dupla interna e +1 apenas para exibir na tela.
  - Terceira numeração: RegistroRodada.duplaVencedora (historico) usa -1 para
    indefinido, 0 para empate e 1/2 para as duplas; EventoAposta.dupla usa 1/2.
    Ao exibir ou registrar no VAR, converta explicitamente a partir de
    rodada/mao (0/1 e EMPATE = -1).
  - Nomes enganosos em Mesa: numeroDaRodada e inicioDaRodada se referem à MÃO,
    não à rodada.
- O fluxo da partida do main.c está em funções static que leem o teclado:
  não reaproveitar. O main_gui.c refaz o fluxo como máquina de estados,
  chamando diretamente os módulos.
- Tudo compila sem avisos com gcc -Wall, em macOS e em Windows (MinGW).
- Comentários em português, explicando o PORQUÊ.
- Antes de cada etapa, explique o plano em poucas linhas e espere meu OK.
- Commits pequenos na branch feature/interface-grafica. Nunca push na main
  nem na develop.

## Arquitetura da tela
- Resolução lógica 1920x1080, desenhada em RenderTexture e ajustada à janela.
- Camadas, de trás para a frente: fundo (panorama com paralaxe), objetos vivos
  do bar, personagens (3 imagens cada: neutro, piscando, falando; respiração e
  piscada feitas por código), mesa, mão do jogador, interface.
- Modo "passa a vez": 4 jogadores humanos no mesmo computador. A câmera é de
  quem joga; os outros 3 aparecem à esquerda, à frente (parceiro) e à direita.
  Entre as vezes, uma tela cobre as cartas.
- Assentos (sentido anti-horário): vista do assento A: direita = (A+1)%4,
  frente = (A+2)%4 (parceiro), esquerda = (A+3)%4. O jogador do assento s usa
  o personagem p(s+1).
- Personagens: 6 imagens cada (vista frente/lado × neutro/piscando/falando).
  O parceiro (frente) usa a vista "frente"; os laterais usam "lado". A arte
  "lado" olha para a DIREITA: na esquerda da tela é usada como está, na
  direita é espelhada. A constante CENA_ARTE_LADO_OLHA_PARA_DIREITA em
  gui/cena.h inverte isso numa linha.
- Recursos carregados por nome fixo em assets/. Se um arquivo não existir,
  desenhar um retângulo colorido com o nome dele, sem travar.
- Texturas com no máximo 4096 px de lado.
- Tela cheia: no macOS o F11 maximiza/restaura (a tela cheia da raylib perde o
  Retina); a tela cheia de verdade é o botão verde ou Ctrl+Cmd+F. PENDENTE:
  testar o F11 (ToggleBorderlessWindowed) e a tela cheia no Windows.
- Fora do escopo: online, chat, emotes, bots, carta escondida, cronômetro,
  mão de onze.

## Decisões de jogabilidade
- somarPontos: refeito no main_gui.c (é static no main.c), marcado com o
  comentário "TEMPORARIO: substituir pela funcao publica do modulo de partida
  (Pessoa 1)". Não alterar src/.
- Truco no modo passa a vez: só quem está na vez pode pedir, antes de jogar
  a carta. Quem responde é o próximo jogador da dupla adversária na ordem de
  jogo: aparece a tela de passa a vez para ele, que escolhe aceitar, correr ou
  aumentar; depois o computador volta para quem pediu.
