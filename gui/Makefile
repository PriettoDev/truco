# Makefile do Truco no Bar do Pato: versao de terminal (entrega do edital) e
# versao grafica (raylib). Os caminhos sao relativos a gui/, entao rode
# "make" de dentro de gui/ ou "make -C gui" da raiz do repositorio.
#
#   make            -> jogo grafico   (output/truco_gui)
#   make terminal   -> jogo terminal  (output/truco)
#   make run        -> compila e abre o jogo grafico
#   make teste      -> compila e roda os testes automaticos (sem raylib)
#   make clean      -> apaga os executaveis
#
# Windows: rode no shell do MSYS2 ou do w64devkit (precisa de sh e mkdir -p).
# Se a raylib estiver em outro lugar: make RAYLIB_INC=... RAYLIB_LIB=...

CC      = gcc
CFLAGS  = -Wall -std=c99 -O2

SRC_DIR = ../src
OUT_DIR = ../output

# O jogo grafico reaproveita todos os modulos de regra, menos o main.c do
# terminal (que tem o seu proprio main e le o teclado).
MODULOS   = $(filter-out $(SRC_DIR)/main.c,$(wildcard $(SRC_DIR)/*.c))
GUI_SRC   = $(wildcard *.c)
CABECALHOS = $(wildcard *.h) $(wildcard $(SRC_DIR)/*.h)

ifeq ($(OS),Windows_NT)
    EXE        = .exe
    # Caminho padrao do instalador oficial da raylib para Windows.
    RAYLIB_DIR ?= C:/raylib/raylib
    RAYLIB_INC ?= $(RAYLIB_DIR)/src
    RAYLIB_LIB ?= $(RAYLIB_DIR)/src
    LDLIBS     = -lraylib -lopengl32 -lgdi32 -lwinmm
    # "make SEM_CONSOLE=1" esconde a janela preta do console na apresentacao.
    ifeq ($(SEM_CONSOLE),1)
        LDLIBS += -mwindows
    endif
else
    EXE =
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        # raylib instalada pelo Homebrew (brew install raylib).
        RAYLIB_DIR ?= $(shell brew --prefix raylib 2>/dev/null)
        RAYLIB_INC ?= $(RAYLIB_DIR)/include
        RAYLIB_LIB ?= $(RAYLIB_DIR)/lib
        LDLIBS     = -lraylib
    else
        # Linux: raylib instalada no sistema.
        RAYLIB_INC ?= /usr/local/include
        RAYLIB_LIB ?= /usr/local/lib
        LDLIBS     = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

TERMINAL = $(OUT_DIR)/truco$(EXE)
GRAFICO  = $(OUT_DIR)/truco_gui$(EXE)
TESTE    = $(OUT_DIR)/teste_partida$(EXE)

# O teste usa so a partida_gui (que nao tem raylib) e os modulos de regra:
# roda em qualquer maquina, sem janela e sem a raylib instalada.
TESTE_SRC = testes/teste_partida.c partida_gui.c

.PHONY: all gui terminal run teste clean

all: gui

gui: $(GRAFICO)

terminal: $(TERMINAL)

# O terminal compila exatamente como no edital: todos os .c de src/ juntos.
$(TERMINAL): $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.h) | $(OUT_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/*.c -o $@

$(GRAFICO): $(MODULOS) $(GUI_SRC) $(CABECALHOS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(RAYLIB_INC) $(MODULOS) $(GUI_SRC) -L$(RAYLIB_LIB) $(LDLIBS) -o $@

$(OUT_DIR):
	mkdir -p $@

run: $(GRAFICO)
	$(GRAFICO)

$(TESTE): $(MODULOS) $(TESTE_SRC) $(CABECALHOS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -I. -I$(SRC_DIR) $(MODULOS) $(TESTE_SRC) -o $@

# Roda sempre (mesmo sem recompilar): o resultado e o que interessa.
teste: $(TESTE)
	$(TESTE)

clean:
	rm -f $(TERMINAL) $(GRAFICO) $(TESTE)
