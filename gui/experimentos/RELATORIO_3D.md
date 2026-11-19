# Experimento: personagens em 3D

Branch `experimento/personagens-3d`. Nada disto volta para a
`feature/interface-grafica` sem aprovação.

## Resumo

Os modelos estáticos do Meshy entram na nossa cena 2D sem dificuldade
técnica. Carregam rápido, o custo de desempenho é pequeno, a textura fica
nítida e não há buracos com o descarte de faces ligado. Antes de ir para o
jogo, falta resolver a **montagem da cena 3D** e a **expressão dos
personagens**:
- o banco;
- um chão em comum;
- a pose das mãos;
- piscar e falar.

As estimativas somam **31 a 45 horas** para os quatro personagens. A entrega
é em 29/11, a 10 dias desta medição.

## Como rodar

```
python3 gui/ferramentas/preparar_modelo.py ~/Documents/BarDoPato_Arte/04_meshy/p1_bruto.glb assets/modelos/p1.glb
python3 gui/ferramentas/preparar_modelo.py ~/Documents/BarDoPato_Arte/04_meshy/p2_bruto.glb assets/modelos/p2.glb
make -C gui teste3d                      # abre a cena; C liga/desliga o descarte
output/teste_3d --medir PASTA            # mede, captura em PASTA e sai
```

## O que foi feito

**`gui/ferramentas/preparar_modelo.py`** (Python com Pillow; leva uns 3 s por modelo):
- tira as texturas de normal e de metálico/rugosidade;
- leva a origem para os pés;
- dilata 16 px as bordas das ilhas da textura de cor;
- deixa o material explicitamente fosco.

| Modelo | Triângulos | Arquivo bruto | Arquivo preparado | Textura de cor |
|---|---|---|---|---|
| p1 | 20.694 | 12,34 MB | 5,08 MB | 2048x2048 RGB |
| p2 | 20.663 | 12,96 MB | 5,17 MB | 2048x2048 RGB |

**`gui/experimentos/teste_3d.c`** (`make -C gui teste3d`, fora do jogo):
- **Composição:** a cena de verdade (panorama, painéis, personagem 2D da
  esquerda, mesa, cartas e HUD) com o p1 na frente e o p2 na direita. Os
  modelos são desenhados numa RenderTexture transparente composta na camada
  dos personagens, e a mesa vem por cima.
- **Câmera 3D casada com a tela lógica:** perspectiva de 30°, olho a 1,2 de
  altura. A uma profundidade d, a altura visível é 2·d·tan(15°), o que
  corresponde a 1080 px. Com isso cada modelo é posto pela caixa
  `CENA_PERSONAGEM_*`.
- **Escala:** o parceiro define a escala do mundo. O p2 usa 1,03 vezes essa
  escala e fica na profundidade em que essa altura ocupa a caixa da direita.
  Resultado: p1 com escala 0,636 a 4,20 de profundidade; p2 com 0,655 a 3,30.
- **Luz:** shader GLSL 330, o mesmo OpenGL 3.3 que a raylib usa no Windows.
  - luz principal quente vinda da esquerda;
  - preenchimento frio e fraco pela direita;
  - ambiente suave;
  - sem especular, porque a textura do Meshy já tem luz pintada;
  - a face de trás usa a normal invertida (`gl_FrontFacing`).
- **Respiração:** escala de 1% em Y a partir dos pés, com ciclo de 3,6 s e
  fases diferentes (0 e 2,1 rad).
- **Textura:** mipmaps (12 níveis) e filtro trilinear.
- **Bordas:** a RenderTexture não tem MSAA, então os modelos são desenhados
  com o dobro da resolução lógica (3840x2160) e reduzidos com filtro bilinear.

## Medições

MacBook M4 Pro, janela de 1600x900 pontos (3200x1800 pixels, Retina),
raylib 6.2. FPS medido sem vsync durante 10 s.

| Medida | Valor |
|---|---|
| Carga do p1 (LoadModel + mipmaps) | 96 ms (153 ms na primeira execução, com o disco frio) |
| Carga do p2 | 91 ms |
| Memória de textura por modelo (2048², RGBA, com mipmaps) | 22,4 MB |
| Memória de textura dos 2 modelos | 44,7 MB (4 modelos: ~89 MB) |
| RenderTexture 3D 3840x2160 (cor + profundidade) | 66 MB |
| **FPS médio, cena inteira** | **228 FPS (4,4 ms por quadro)** |

O jogo roda com vsync (60 FPS), então sobra muita margem nesta máquina.
**Não medi no Windows.** Num notebook com GPU integrada, o que mais pesa é a
RenderTexture com superamostragem. Se faltar desempenho, há três saídas:
- `SUPERAMOSTRAGEM 1`, que corta 75% dos pixels;
- textura de 1024, com 5,6 MB por modelo;
- desenhar direto, sem RenderTexture, usando o MSAA da janela.

## Faces de trás (descarte)

Comparei a imagem 3D com o descarte ligado e desligado, contando os pixels
que mudam nas regiões ampliadas:
- frente (p1): **17 pixels**;
- direita (p2): **81 pixels**.

São fios soltos de borda, invisíveis no tamanho do jogo, e não buracos. No
boné, no laço do lenço e nas cavas da regata não aparece nada vazado nas
capturas. Conclusão: **pode deixar o descarte ligado**, que é mais barato.
O experimento mantém a opção (tecla C, `descarteLigado`) e o shader já trata
a face de trás, caso um modelo futuro precise.

## Capturas

Pasta `capturas_3d/`, todas feitas com `TakeScreenshot` de dentro do programa.

- Cena inteira: `cena_descarte_ligado.png` e `cena_descarte_desligado.png`
- Zoom no p1 (frente): `zoom_p1_frente_descarte_ligado.png` e `zoom_p1_frente_descarte_desligado.png`
- Zoom no p2 (direita): `zoom_p2_direita_descarte_ligado.png` e `zoom_p2_direita_descarte_desligado.png`

A mesa e os painéis ainda são marcadores provisórios. A mesa é
**semitransparente**, por isso as pernas aparecem através dela. Com a arte
opaca, a mesa esconde tudo da cintura para baixo.

## O que as capturas mostram

1. **Sentados no ar.** Não há banco nem cadeira. Hoje a mesa esconde isso de
   frente, mas no da direita a perna e o quadril aparecem do lado da mesa.
2. **Sem chão em comum.** As caixas 2D foram desenhadas para imagens planas:
   - os pés do p1 ficam em y = 0,76 no mundo;
   - os do p2 ficam em y = 0,35;
   - num banco de verdade, os dois precisam estar no mesmo chão.

   O jeito certo é montar o lugar de cada um em coordenadas 3D (chão, banco e
   tampo da mesa a uma altura fixa) e conferir o resultado na tela, e não o
   contrário.
3. **Pose das mãos.** O Meshy entregou os braços esticados para a frente, com
   as mãos abertas no ar. Na mesa deveriam estar apoiadas no tampo ou
   segurando o leque de cartas. A pose vem do modelo, e sem esqueleto não dá
   para dobrar em código.
4. **Tamanho do da direita.** Com a caixa 2D da direita (760 px de altura), a
   cabeça do p2 fica grande perto do parceiro. É perspectiva correta, mas a
   caixa foi pensada para uma arte de perfil. Com o layout em 3D (item 2),
   esse tamanho passa a ser uma decisão de câmera.
5. **Qualidade.** A textura dilatada não mostra costura, a luz quente da
   esquerda dá volume e o estilo combina com o bar. A troca para 3D vale
   visualmente.

## O que faltaria para os 4 personagens no jogo

| # | Tarefa | Horas |
|---|---|---|
| 1 | Gerar p3 e p4 no Meshy e preparar com `preparar_modelo.py` (incluir sentado e mãos na mesa no pedido ao Meshy) | 2 a 3 |
| 2 | **Banco ou cadeira** para cada um: modelo do Meshy (prop) ou caixa texturizada em código; altura fixa do assento | 3 a 5 |
| 3 | Layout em coordenadas 3D: chão, banco, tampo da mesa e câmera por posição (frente, direita, esquerda); conferência automática dos rostos contra `CENA_ROSTO_*` | 4 a 6 |
| 4 | Pose: gerar de novo com as mãos apoiadas ou com cartas (depende do Meshy aceitar a pose) | 2 a 4 |
| 5 | Integrar no `cena.c`: camada 3D com volta para a imagem 2D se o `.glb` faltar (regra dos recursos do `CLAUDE.md`), carregar os 4 modelos uma vez e reposicionar a cada troca de vez (a câmera gira com o jogador) | 4 a 6 |
| 6 | Esquerda e direita: só giro do modelo (+70° e −70°), sem espelhar imagem; ajustar `CENA_ARTE_LADO_OLHA_PARA_DIREITA` (perde o sentido) | 1 |
| 7 | **Piscar e falar.** O modelo é estático, sem blend shapes e sem esqueleto. Opções: (a) pálpebras e boca 2D desenhadas por cima, presas à posição projetada da cabeça; (b) trocar a textura do rosto (exige textura por expressão); (c) só respiração e um balanço leve da cabeça | (a) 8 a 12; (c) 2 |
| 8 | Mesa: arte opaca na perspectiva da câmera 3D (ou mesa 3D simples) cobrindo pernas e banco | 2 a 4 |
| 9 | Teste no Windows: shader GLSL 330 em GPU Intel/AMD, desempenho, tamanho da RenderTexture e da textura | 3 a 4 |
| 10 | Testes visuais e capturas de todas as telas (cobertura, truco e fim de partida usam a vista de frente do personagem) | 2 |
| | **Total** | **31 a 45** (com 7a) / **25 a 35** (com 7c) |

### Riscos

- **Prazo:** a entrega é em 29/11, a 10 dias. As Etapas 4 a 7 (VAR,
  animações 2D, sons) também disputam esse tempo.
- **Piscar e falar** é o item mais caro e o que mais muda a sensação de
  personagem vivo. Na arquitetura atual (3 imagens por expressão), isso já
  vem pronto com a arte 2D.
- **Windows:** nada foi medido lá, e a apresentação provavelmente será
  nele.
