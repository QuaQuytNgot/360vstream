#ifndef VIEWPORT_PREDICTION_H
#define VIEWPORT_PREDICTION_H
#include <vstream/define.h>
#include <vstream/tile_selection.h>

typedef struct viewport_prediction_t viewport_prediction_t;

struct viewport_prediction_t
{
  float *yaw_history;
  float *pitch_history;
  int   *timestamps;
  int    current_index;
  int    sample_count;
  int    next_timestamp;
  int    history_size;

  RET (*vpes_post)(viewport_prediction_t *, float *, float *);
};

RET   vpes_init(viewport_prediction_t *,
                float             *yaw_history,
                float             *pitch_history,
                int               *timestamps,
                int                current_index,
                int                sample_count,
                int                next_timestamp,
                int                history_size;
                VIEWPORT_ESTIMATOR type);

RET   calculate_linear_regression(float *y_values,
                                  int   *x_values,
                                  int    n,
                                  float *slope,
                                  float *intercept);
float wrap_angle_360(float angle);
float clamp_pitch(float pitch);
void  adjust_yaw_for_wrapping(float *yaw_values, int n);
RET vpes_legr(viewport_prediction_t *vpes, float *yaw, float *pitch);

#endif