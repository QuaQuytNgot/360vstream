#ifndef TILE_SELECTION_H
#define TILE_SELECTION_H

#include "define.h"

typedef struct tile_selection_t tile_selection_t;

struct tile_selection_t
{
    void (*select_viewport)(
        double yaw, double pitch,
        int *tile_id, int *num_tiles);
};

int intersects(float a_min,
               float a_max,
               float b_min,
               float b_max);
void define_viewport(float yaw,
                     float pitch,
                     int *tile,
                     int *num_tiles);

// should contain an init function to reset *tile every loop, a struct to control *tile, *num_tile

#endif