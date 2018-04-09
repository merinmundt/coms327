#include "dungeon.h"




void players_t::fillPairArray(pair_xy_t *arr)
{
    for(int i = 0;i < num_chars;i++){
        if(gameCharacters[i].dead){
            arr[i] = pair_xy_t(0,0);
        }else{
            arr[i] = pair_xy_t(gameCharacters[i].pos.x, gameCharacters[i].pos.y);
        }
    }
}
