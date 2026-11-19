"""Prepara um modelo bruto do Meshy (GLB estatico) para o jogo.

Uso (da raiz do repositorio):
    python3 gui/ferramentas/preparar_modelo.py ENTRADA.glb assets/modelos/p1.glb

O que faz, e por que:
  1. Remove as texturas de normal e de metalico/rugosidade. O shader do jogo
     so usa a cor; as outras duas ocupariam ~2/3 da memoria de textura e do
     arquivo sem mudar nada na tela.
  2. Move a origem para os pes: menor Y = 0, centrado em X e Z. Assim o
     modelo e posicionado pelo ponto onde encosta no chao/banco, e a
     respiracao (escala em Y) cresce para cima, sem afundar na mesa. A
     translacao vai direto nos vertices, porque a raylib ignora parte da
     hierarquia de nos do glTF.
  3. Dilata as bordas das ilhas da textura de cor. Fora das ilhas o Meshy
     deixa preto; com mipmaps, os niveis menores misturam esse preto com a
     borda e aparece uma costura escura no modelo. A cor da borda e
     empurrada para fora, pixel a pixel, por DILATACAO_PX pixels.
  4. Grava o GLB novo e imprime triangulos, tamanho e texturas antes e depois.

Precisa do Pillow (pip install --user pillow), so para ler e gravar o PNG
e para a dilatacao. O resto e Python puro.
"""
import io
import json
import os
import struct
import sys

from PIL import Image, ImageChops, ImageDraw

GLB_MAGICO = 0x46546C67   # "glTF"
BLOCO_JSON = 0x4E4F534A
BLOCO_BIN = 0x004E4942

# 2048 px dividido por 2 a cada nivel de mipmap: com 16 px de folga, ate o
# nivel 4 (128 px) a borda ainda tem pelo menos 1 px de cor certa.
DILATACAO_PX = 16

FLOAT = 5126


def ler_glb(caminho):
    dados = open(caminho, 'rb').read()
    magico, versao, _ = struct.unpack_from('<III', dados, 0)
    if magico != GLB_MAGICO or versao != 2:
        raise SystemExit(f'{caminho}: nao e um GLB 2.0')
    pos = 12
    gltf, binario = None, b''
    while pos < len(dados):
        tamanho, tipo = struct.unpack_from('<II', dados, pos)
        bloco = dados[pos + 8:pos + 8 + tamanho]
        if tipo == BLOCO_JSON:
            gltf = json.loads(bloco)
        elif tipo == BLOCO_BIN:
            binario = bloco
        pos += 8 + tamanho
    return gltf, binario


def gravar_glb(caminho, gltf, binario):
    # Os dois blocos precisam de tamanho multiplo de 4: o JSON completa com
    # espacos e o binario com zeros (especificacao do GLB).
    texto = json.dumps(gltf, separators=(',', ':')).encode('utf-8')
    texto += b' ' * (-len(texto) % 4)
    binario += b'\0' * (-len(binario) % 4)
    total = 12 + 8 + len(texto) + 8 + len(binario)
    with open(caminho, 'wb') as f:
        f.write(struct.pack('<III', GLB_MAGICO, 2, total))
        f.write(struct.pack('<II', len(texto), BLOCO_JSON) + texto)
        f.write(struct.pack('<II', len(binario), BLOCO_BIN) + binario)


def bytes_da_view(gltf, binario, indice):
    v = gltf['bufferViews'][indice]
    inicio = v.get('byteOffset', 0)
    return binario[inicio:inicio + v['byteLength']]


def ler_acessor(gltf, binario, indice):
    """Lista de tuplas (VEC2/VEC3) ou de inteiros (SCALAR)."""
    a = gltf['accessors'][indice]
    v = gltf['bufferViews'][a['bufferView']]
    comp = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3}[a['type']]
    fmt = {5126: 'f', 5125: 'I', 5123: 'H'}[a['componentType']]
    tam = struct.calcsize(fmt)
    passo = v.get('byteStride', comp * tam)
    inicio = v.get('byteOffset', 0) + a.get('byteOffset', 0)
    saida = []
    for i in range(a['count']):
        valores = struct.unpack_from('<' + fmt * comp, binario, inicio + i * passo)
        saida.append(valores if comp > 1 else valores[0])
    return saida


def info(gltf, binario, rotulo):
    prim = gltf['meshes'][0]['primitives'][0]
    tri = gltf['accessors'][prim['indices']]['count'] // 3
    pos = gltf['accessors'][prim['attributes']['POSITION']]
    imagens = [(im.get('name', '?'), gltf['bufferViews'][im['bufferView']]['byteLength'])
               for im in gltf.get('images', [])]
    print(f'  {rotulo}: {tri} triangulos | binario {len(binario) / 1e6:.2f} MB | '
          f'Y {pos["min"][1]:+.3f} a {pos["max"][1]:+.3f} | X {pos["min"][0]:+.3f} a {pos["max"][0]:+.3f} | '
          f'Z {pos["min"][2]:+.3f} a {pos["max"][2]:+.3f}')
    for nome, tam in imagens:
        print(f'      textura {nome}: {tam / 1e6:.2f} MB')


def mascara_das_ilhas(uvs, indices, largura, altura):
    """Pinta de branco os pixels cobertos por algum triangulo no espaco UV.
    No glTF a UV (0,0) e o canto de cima a esquerda da imagem, como as linhas
    do PNG, entao nao ha inversao de V."""
    mascara = Image.new('L', (largura, altura), 0)
    pintor = ImageDraw.Draw(mascara)
    for t in range(0, len(indices), 3):
        pontos = [(uvs[i][0] * largura, uvs[i][1] * altura) for i in indices[t:t + 3]]
        # outline tambem: triangulos finos de 1 px nao sumiriam da mascara.
        pintor.polygon(pontos, fill=255, outline=255)
    return mascara


def dilatar(cor, mascara, passos):
    """A cada passo, cada pixel fora da mascara que tem um vizinho dentro
    copia a cor desse vizinho e entra na mascara. Feito com deslocamentos da
    imagem inteira (rapido, em C dentro do Pillow), sem numpy."""
    direcoes = ((1, 0), (-1, 0), (0, 1), (0, -1))
    for _ in range(passos):
        for dx, dy in direcoes:
            # ImageChops.offset da a volta na borda: a borda oposta da textura
            # tambem e fora das ilhas na pratica (o Meshy deixa margem).
            vizinho = ImageChops.offset(mascara, dx, dy)
            novos = ImageChops.subtract(vizinho, mascara)    # vizinho dentro, eu fora
            cor = Image.composite(ImageChops.offset(cor, dx, dy), cor, novos)
            mascara = ImageChops.lighter(mascara, vizinho)
    return cor


def preparar(entrada, saida):
    gltf, binario = ler_glb(entrada)
    print(f'{entrada}')
    info(gltf, binario, 'antes ')

    if len(gltf['meshes']) != 1 or len(gltf['meshes'][0]['primitives']) != 1:
        raise SystemExit('esperava 1 malha com 1 primitiva (modelo estatico do Meshy)')
    prim = gltf['meshes'][0]['primitives'][0]
    material = gltf['materials'][prim['material']]
    pbr = material['pbrMetallicRoughness']
    idx_pos = prim['attributes']['POSITION']
    textura_cor = gltf['textures'][pbr['baseColorTexture']['index']]
    imagem_cor = gltf['images'][textura_cor['source']]

    # --- 2. origem nos pes
    posicoes = ler_acessor(gltf, binario, idx_pos)
    mn = [min(p[k] for p in posicoes) for k in range(3)]
    mx = [max(p[k] for p in posicoes) for k in range(3)]
    desloc = (-(mn[0] + mx[0]) / 2, -mn[1], -(mn[2] + mx[2]) / 2)
    novas = [(p[0] + desloc[0], p[1] + desloc[1], p[2] + desloc[2]) for p in posicoes]
    bloco_pos = b''.join(struct.pack('<fff', *p) for p in novas)

    # --- 3. dilatacao da textura de cor
    png = bytes_da_view(gltf, binario, imagem_cor['bufferView'])
    cor = Image.open(io.BytesIO(png))
    modo_original = cor.mode
    cor = cor.convert('RGBA' if 'A' in cor.mode else 'RGB')
    uvs = ler_acessor(gltf, binario, prim['attributes']['TEXCOORD_0'])
    indices = ler_acessor(gltf, binario, prim['indices'])
    mascara = mascara_das_ilhas(uvs, indices, cor.width, cor.height)
    cobertura = sum(mascara.histogram()[255:]) / (cor.width * cor.height)
    cor = dilatar(cor, mascara, DILATACAO_PX)
    saida_png = io.BytesIO()
    cor.save(saida_png, format='PNG', optimize=True)
    print(f'  textura de cor {cor.width}x{cor.height} {modo_original}: ilhas cobrem '
          f'{cobertura * 100:.1f}% | dilatada {DILATACAO_PX} px')

    # --- 1. remonta o binario so com o que fica: posicoes novas, normais,
    # UVs, indices e a textura de cor. As outras imagens somem.
    novo_bin = b''
    novas_views = []

    def anexar(dados, alvo=None):
        nonlocal novo_bin
        novo_bin += b'\0' * (-len(novo_bin) % 4)    # cada view alinhada em 4
        view = {'buffer': 0, 'byteOffset': len(novo_bin), 'byteLength': len(dados)}
        if alvo:
            view['target'] = alvo
        novo_bin += dados
        novas_views.append(view)
        return len(novas_views) - 1

    novos_acessores = []

    def copiar_acessor(indice, dados=None):
        a = dict(gltf['accessors'][indice])
        v = gltf['bufferViews'][a['bufferView']]
        if dados is None:
            dados = bytes_da_view(gltf, binario, a['bufferView'])
        a['bufferView'] = anexar(dados, v.get('target'))
        a.pop('byteOffset', None)
        novos_acessores.append(a)
        return len(novos_acessores) - 1

    a_pos = copiar_acessor(idx_pos, bloco_pos)
    novos_acessores[a_pos]['min'] = [min(p[k] for p in novas) for k in range(3)]
    novos_acessores[a_pos]['max'] = [max(p[k] for p in novas) for k in range(3)]
    atributos = {'POSITION': a_pos}
    for nome in ('NORMAL', 'TEXCOORD_0'):
        if nome in prim['attributes']:
            atributos[nome] = copiar_acessor(prim['attributes'][nome])
    a_ind = copiar_acessor(prim['indices'])
    v_img = anexar(saida_png.getvalue())

    gltf['accessors'] = novos_acessores
    gltf['bufferViews'] = novas_views
    gltf['buffers'] = [{'byteLength': len(novo_bin)}]
    gltf['images'] = [{'bufferView': v_img, 'mimeType': 'image/png', 'name': 'cor'}]
    gltf['textures'] = [{'sampler': 0, 'source': 0}]
    prim['attributes'] = atributos
    prim['indices'] = a_ind
    material.pop('normalTexture', None)
    pbr.pop('metallicRoughnessTexture', None)
    pbr['baseColorTexture'] = {'index': 0}
    # Sem a textura de metalico, o padrao do glTF seria metalico 1,0 (espelho)
    # em outros visualizadores; deixa explicito que e fosco.
    pbr['metallicFactor'] = 0.0
    pbr['roughnessFactor'] = 1.0

    os.makedirs(os.path.dirname(saida) or '.', exist_ok=True)
    gravar_glb(saida, gltf, novo_bin)
    info(gltf, novo_bin, 'depois')
    print(f'  arquivo: {os.path.getsize(entrada) / 1e6:.2f} MB -> {os.path.getsize(saida) / 1e6:.2f} MB  ({saida})')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    preparar(sys.argv[1], sys.argv[2])
