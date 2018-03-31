#include "dungeon.h"




void players_t::fillPairArray(pair_xy_t *arr)
{
    for(int i = 0;i < num_chars;i++){
        arr[i] = pair_xy_t(gameCharacters[i].pos.x, gameCharacters[i].pos.y);
    }
}
