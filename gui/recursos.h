#ifndef RECURSOS_H
#define RECURSOS_H

/*
 * recursos.h - imagens e fontes carregadas por nome fixo em assets/.
 *
 * Por que nome fixo: quem faz a arte so precisa salvar o arquivo com o nome
 * combinado (lista em assets/LEIAME.md) e o jogo passa a usa-lo, sem mexer
 * no codigo. Se um arquivo faltar, o jogo desenha um retangulo colorido com
 * o nome dele no lugar e segue funcionando.
 */
#include "raylib.h"

/* A ordem dos personagens permite calcular o id (ver recurso_personagem):
   6 imagens por personagem, primeiro as 3 de frente, depois as 3 de lado. */
typedef enum {
    REC_FUNDO_PANORAMA,
    REC_FUNDO_MESA,

    REC_PERSONAGENS_INICIO,                         /* p1_frente_neutro */
    REC_PERSONAGENS_FIM = REC_PERSONAGENS_INICIO + 4 * 6 - 1,

    REC_PAINEL_LOUSA,
    REC_PAINEL_TV_VAR,
    REC_PAINEL_MURAL,
    REC_PAINEL_CADERNETA,

    REC_UI_LOGO,
    REC_UI_VERSO_CARTA,

    REC_TOTAL
} RecursoId;

typedef enum { VISTA_FRENTE, VISTA_LADO } Vista;
typedef enum { EXPRESSAO_NEUTRO, EXPRESSAO_PISCANDO, EXPRESSAO_FALANDO } Expressao;

typedef enum {
    FONTE_TITULO,
    FONTE_TEXTO,
    FONTE_NAIPES,
    FONTE_TOTAL
} FonteId;

/* Carrega tudo de uma vez (chamar depois do InitWindow) e avisa no terminal
   cada arquivo que faltar. Nunca encerra o jogo por falta de arquivo. */
void recursos_carregar(void);
void recursos_descarregar(void);

/* personagem: 1 a 4 (o jogador do assento s usa o personagem s+1). */
RecursoId recurso_personagem(int personagem, Vista vista, Expressao expressao);

int recurso_existe(RecursoId id);
const char *recurso_nome(RecursoId id);

/* Tamanho original da imagem em pixels; {0,0} se o arquivo faltar. Serve
   para quem precisa manter a proporcao (ex.: o panorama ajustado pela altura). */
Vector2 recurso_tamanho(RecursoId id);

/* Desenha a imagem esticada no retangulo (coordenadas logicas). espelhado=1
   inverte na horizontal (personagem de lado na posicao direita). Sem o
   arquivo, desenha o marcador provisorio no mesmo retangulo. */
void recurso_desenhar(RecursoId id, Rectangle destino, int espelhado, Color tinta);

/* Mesma ideia, girada em graus em torno do centro (versos em leque, cartas
   dos jogadores laterais). O marcador provisorio gira junto. */
void recurso_desenhar_girado(RecursoId id, Vector2 centro, Vector2 tamanho,
                             float graus, Color tinta);

/* Sempre devolve uma fonte utilizavel: a do arquivo ou a padrao da raylib. */
Font recurso_fonte(FonteId id);
int recurso_fonte_existe(FonteId id);

#endif
