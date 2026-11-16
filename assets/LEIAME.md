# Recursos do Truco no Bar do Pato

O jogo procura cada arquivo pelo nome exato abaixo. Se um arquivo faltar,
aparece um retângulo colorido com o nome dele no lugar, e o terminal avisa.
Basta salvar o arquivo com o nome certo para ele aparecer no jogo.

Regras: PNG com transparência; no máximo 4096 px de lado (acima disso o
arquivo é recusado e o terminal avisa).

## fundo/
- `bar_panorama.png`: panorama do bar, mais largo que 16:9. O jogo ajusta
  pela altura; a largura que sobra serve para a paralaxe. Já deve trazer
  pintados a lousa, a TV, o mural e a caderneta.
- `mesa.png`: tampo da mesa.

## personagens/ (N = 1 a 4)
Seis imagens por personagem, todas do mesmo tamanho:
- `pN_frente_neutro.png`, `pN_frente_piscando.png`, `pN_frente_falando.png`:
  vista de frente (usada para o parceiro, sentado à frente).
- `pN_lado_neutro.png`, `pN_lado_piscando.png`, `pN_lado_falando.png`:
  perfil olhando para a DIREITA. Na posição esquerda o jogo espelha a imagem.

## paineis/
Áreas sobre objetos já pintados no panorama:
- `lousa.png` (placar), `tv_var.png` (VAR), `mural.png` (ranking),
  `caderneta.png`.

## ui/
- `logo.png`
- `verso_carta.png`: só o verso. As faces das cartas são desenhadas pelo jogo.
- `fonte_titulo.ttf`, `fonte_texto.ttf`: precisam ter os acentos do
  português (á, ã, ç, ê, ó...).
- `fonte_naipes.ttf`: precisa ter os símbolos ♦ ♠ ♥ ♣.
  Se uma fonte faltar, o jogo usa a fonte padrão da raylib.

## audio/
Vazio por enquanto.
