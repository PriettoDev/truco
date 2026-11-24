#ifndef SALVAMENTO_H
#define SALVAMENTO_H

#include "mesa.h"
#include "historico.h"

#define ARQUIVO_PARTIDA "partida.sav"

// Retorna 1 se há um arquivo válido marcado como partida em andamento
int salvamento_disponivel(const char *caminho);

// Grava mesa, assentos/ordem e VAR sem persistir ponteiros ou structs inteiras
int salvarPartida(const char *caminho, const Mesa *mesa, const tp_listad *historico);

// Carrega uma partida válida em andamento e reconstrói a lista do VAR
int carregarPartida(const char *caminho, Mesa *mesa, tp_listad **historico);

#endif