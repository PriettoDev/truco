#include <stddef.h>
#include "pontuacao.h"

int pontuacao_somar_mao(Mesa *mesa, const Mao *mao){ //Aqui eu uso o const Mao, pois assim eu não altero o valor original da mao. Já na questão da mesa eu quero mexer noo placar real, dai uso um ponteiro para ele, alterando o valor original
    int *placar;
    int antes;

    if(mesa==NULL || mao==NULL)
        return 0;
    
    if(mao->dupla_vencedora != 0 && mao->dupla_vencedora != 1) //essa função serve para ccaso alguma dupla fique com a mao empatada ou a rodada fique sem vencedor
        return 0;

    if(mao->pontos_ganhos <= 0)
        return 0;
    
    if(mao->dupla_vencedora == 0){
        placar = &mesa->pontosDupla1;
    } else{
        placar = &mesa->pontosDupla2;        
    }

    antes = *placar;
    *placar += mao->pontos_ganhos;

    if(*placar > PONTOS_PARA_VENCER)
        *placar = PONTOS_PARA_VENCER;
    
    return *placar - antes;
}


int pontuacao_dupla_vencedora(const Mesa *mesa){
    if(mesa == NULL)
        return 0;

    if(mesa->pontosDupla1 >= PONTOS_PARA_VENCER)
        return 1;

    if(mesa->pontosDupla2 >= PONTOS_PARA_VENCER)
        return 2;

    return 0;
}

int pontuacao_partida_terminou(const Mesa *mesa){
    return pontuacao_dupla_vencedora(mesa) != 0;
}