"""Confere se as fontes de assets/ui/ tem todos os caracteres que o jogo pede.

Uso (da raiz do repositorio):
    python3 gui/ferramentas/conferir_fontes.py            # confere assets/ui/
    python3 gui/ferramentas/conferir_fontes.py outra/pasta

Por que existe: o LoadFontEx da raylib nao avisa de forma clara quando falta
um caractere; a letra so aparece como "?" na tela. Este script le a tabela
cmap do .ttf direto (formatos 4 e 12), sem instalar nada, e lista o que falta.

IMPORTANTE: as listas abaixo precisam ficar iguais a montar_codepoints() em
gui/recursos.c. Mudou la, mude aqui.

Saida: 0 se nao falta nenhum caractere visivel (fora os ACEITOS), 1 se falta
ou se der erro.
"""
import os
import struct
import sys

ASCII = list(range(32, 127))
TEXTO = ASCII + list(range(0xA0, 0x100)) + [0x2014, 0x2026, 0x201C, 0x201D]
NAIPES = ASCII + [0x2660, 0x2663, 0x2665, 0x2666]

FONTES = (
    ('fonte_titulo.ttf', TEXTO),
    ('fonte_texto.ttf', TEXTO),
    ('fonte_naipes.ttf', NAIPES),
)

# Nao desenham nada: listados a parte para nao parecer falta de letra.
INVISIVEIS = (0xA0, 0xAD)   # espaco sem quebra, hifen invisivel

# Faltas ja avaliadas e aceitas (nao usadas em portugues). Listadas a parte
# para o script nao acusar erro para sempre por causa delas.
ACEITOS = {
    'fonte_titulo.ttf': (0xAF,),   # macron, ausente na Lilita One
}


def codepoints_da_fonte(caminho):
    """Devolve o conjunto de caracteres que a fonte realmente desenha."""
    with open(caminho, 'rb') as arquivo:
        dados = arquivo.read()
    quantas = struct.unpack('>H', dados[4:6])[0]
    tabelas = {}
    for i in range(quantas):
        tag, _, inicio, _ = struct.unpack('>4sIII', dados[12 + 16 * i:28 + 16 * i])
        tabelas[tag] = inicio
    if b'cmap' not in tabelas:
        raise ValueError('sem tabela cmap')
    cmap = tabelas[b'cmap']
    subtabelas = struct.unpack('>H', dados[cmap + 2:cmap + 4])[0]
    encontrados = set()
    for i in range(subtabelas):
        plataforma, codificacao, inicio = struct.unpack(
            '>HHI', dados[cmap + 4 + 8 * i:cmap + 12 + 8 * i])
        # So as subtabelas Unicode (Windows ou Unicode); as de Mac sao legado.
        if not ((plataforma == 3 and codificacao in (1, 10)) or plataforma == 0):
            continue
        s = cmap + inicio
        formato = struct.unpack('>H', dados[s:s + 2])[0]
        if formato == 4:
            encontrados |= _formato4(dados, s)
        elif formato == 12:
            encontrados |= _formato12(dados, s)
    return encontrados


def _formato4(dados, s):
    """Formato 4: faixas de caracteres de 16 bits (o mais comum)."""
    tam = struct.unpack('>H', dados[s + 6:s + 8])[0]
    n = tam // 2
    fins = struct.unpack('>%dH' % n, dados[s + 14:s + 14 + tam])
    inicios = struct.unpack('>%dH' % n, dados[s + 16 + tam:s + 16 + 2 * tam])
    deltas = struct.unpack('>%dh' % n, dados[s + 16 + 2 * tam:s + 16 + 3 * tam])
    pos_desloc = s + 16 + 3 * tam
    deslocs = struct.unpack('>%dH' % n, dados[pos_desloc:pos_desloc + tam])
    achados = set()
    for k in range(n):
        for c in range(inicios[k], fins[k] + 1):
            if c == 0xFFFF:
                continue
            if deslocs[k] == 0:
                glifo = (c + deltas[k]) & 0xFFFF
            else:
                p = pos_desloc + 2 * k + deslocs[k] + 2 * (c - inicios[k])
                glifo = struct.unpack('>H', dados[p:p + 2])[0]
                if glifo:
                    glifo = (glifo + deltas[k]) & 0xFFFF
            if glifo:              # glifo 0 = ".notdef", ou seja, nao tem
                achados.add(c)
    return achados


def _formato12(dados, s):
    """Formato 12: grupos de 32 bits (fontes com caracteres alem de U+FFFF)."""
    grupos = struct.unpack('>I', dados[s + 12:s + 16])[0]
    achados = set()
    for k in range(grupos):
        a, b, glifo = struct.unpack('>III', dados[s + 16 + 12 * k:s + 28 + 12 * k])
        for c in range(a, b + 1):
            if glifo + (c - a):
                achados.add(c)
    return achados


def main():
    raiz = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    pasta = sys.argv[1] if len(sys.argv) > 1 else os.path.join(raiz, 'assets', 'ui')
    falhou = False
    for nome, pedidos in FONTES:
        caminho = os.path.join(pasta, nome)
        try:
            tem = codepoints_da_fonte(caminho)
        except (OSError, ValueError, struct.error) as erro:
            print('%-17s ERRO: %s' % (nome, erro))
            falhou = True
            continue
        faltam = [c for c in pedidos if c not in tem]
        aceitos = [c for c in faltam if c in ACEITOS.get(nome, ())]
        invisiveis = [c for c in faltam if c in INVISIVEIS]
        visiveis = [c for c in faltam if c not in INVISIVEIS and c not in aceitos]
        print('%-17s pedidos %3d | faltam %d: %s | aceitos: %s | invisiveis: %s' % (
            nome, len(pedidos), len(visiveis),
            ' '.join('U+%04X(%s)' % (c, chr(c)) for c in visiveis) or '-',
            ' '.join('U+%04X(%s)' % (c, chr(c)) for c in aceitos) or '-',
            ' '.join('U+%04X' % c for c in invisiveis) or '-'))
        falhou = falhou or bool(visiveis)
    return 1 if falhou else 0


if __name__ == '__main__':
    sys.exit(main())
