#include "vstream/viewport_predition.h"
#include <stdlib.h>

RET viewport_prediction_init(viewport_prediction_t *vpes,
                             float                 *yaw_history,
                             float                 *pitch_history,
                             int                   *timestamps,
                             int                    current_index,
                             int                    sample_count,
                             int                    next_timestamp,
                             int                    history_size,
                             VIEWPORT_ESTIMATOR     type)
{

  if (vpes = NULL_POINTER)
  {
    return RET_FAIL;
  }

  vpes->yaw_history    = yaw_history;
  vpes->pitch_history  = pitch_history;
  vpes->timestamps     = timestamps;
  vpes->current_index  = current_index;
  vpes->sample_count   = sample_count;
  vpes->next_timestamp = next_timestamp;
  vpes->history_size   = history_size;

  switch (type)
  {
  case VIEWPORT_ESTIMATOR_LEGR:
    vpes->vpes_post = vpes_legr;
    break;

  default:
    break;
  }
  return RET_SUCCESS;
}

RET calculate_linear_regression(float *y_values,
                                int   *x_values,
                                int    n,
                                float *slope,
                                float *intercept)
{
  if (n < 2 || y_values == NULL_POINTER ||
      x_values == NULL_POINTER || slope == NULL_POINTER ||
      intercept == NULL_POINTER)
  {
    return RET_FAIL;
  }

  float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;

  for (int i = 0; i < n; i++)
  {
    sum_x += x_values[i];
    sum_y += y_values[i];
    sum_xy += x_values[i] * y_values[i];
    sum_x2 += x_values[i] * x_values[i];
  }

  float denominator = n * sum_x2 - sum_x * sum_x;
  if (fabs(denominator) < 1e-10)
  {
    return RET_FAIL;
  }

  *slope     = (n * sum_xy - sum_x * sum_y) / denominator;
  *intercept = (sum_y - (*slope) * sum_x) / n;

  return RET_SUCCESS;
}

float wrap_angle_360(float angle)
{
  while (angle < 0)
    angle += 360;
  while (angle >= 360)
    angle -= 360;
  return angle;
}

float clamp_pitch(float pitch)
{
  if (pitch > 90)
    return 90;
  if (pitch < -90)
    return -90;
  return pitch;
}

void adjust_yaw_for_wrapping(float *yaw_values, int n)
{
  if (n < 2 || yaw_values == NULL_POINTER)
    return;

  float min_yaw = yaw_values[0], max_yaw = yaw_values[0];
  for (int i = 1; i < n; i++)
  {
    if (yaw_values[i] < min_yaw)
      min_yaw = yaw_values[i];
    if (yaw_values[i] > max_yaw)
      max_yaw = yaw_values[i];
  }

  if (max_yaw - min_yaw > 180)
  {
    for (int i = 0; i < n; i++)
    {
      if (yaw_values[i] < 180)
      {
        yaw_values[i] += 360;
      }
    }
  }
}

RET tile_selection_vpes_legr(viewport_prediction_t *vpes,
                             float                 *yaw,
                             float                 *pitch)
{
  if (yaw == NULL_POINTER || pitch == NULL_POINTER)
  {
    return RET_FAIL;
  }
}