#ifndef TILE_SELECTION_H
#define TILE_SELECTION_H

#include <vstream/define.h>

typedef struct tile_selection_t tile_selection_t;

struct tile_selection_t
{
  void (*select_viewport)(float yaw,
                          float pitch,
                          int  *tile_id,
                          int  *num_tiles);
  RET (*vpes)(float *, float *, int, int);
};

/* a function to define vp yaw, pitch; then call defin_viewport() to
define tile in the viewport */

RET   tile_selection_init(tile_selection_t *tile, VIEWPORT_ESTIMATOR type);

// function for vp prediction using linear regression
// RET   calculate_linear_regression(float *y_values,
//                                   int   *x_values,
//                                   int    n,
//                                   float *slope,
//                                   float *intercept);
// float wrap_angle_360(float angle);
// float clamp_pitch(float pitch);
// void  adjust_yaw_for_wrapping(float *yaw_values, int n);
// RET   tile_selection_vpes_legr(float *yaw,
//                                float *pitch,
//                                int    yaw_history,
//                                int    pitch_hisory);

int   intersects(float a_min, float a_max, float b_min, float b_max);
void  define_viewport(float yaw,
                      float pitch,
                      int **tile,
                      int  *num_tiles);

// struct to control *tile, *num_tile

#endif