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
    return RET_FAIL;
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

void add_viewport_sample(viewport_prediction_t *vpes,
                         float                  yaw,
                         float                  pitch)
{
  if (vpes == NULL_POINTER || vpes->yaw_history == NULL_POINTER ||
      vpes->pitch_history == NULL_POINTER ||
      vpes->timestamps == NULL_POINTER)
  {
    return;
  }

  vpes->yaw_history[vpes->current_index]   = yaw;
  vpes->pitch_history[vpes->current_index] = pitch;
  vpes->timestamps[vpes->current_index]    = vpes->next_timestamp++;

  vpes->current_index =
      (vpes->current_index + 1) % vpes->history_size;
  if (vpes->sample_count < vpes->history_size)
  {
    vpes->sample_count++;
  }
}

RET vpes_legr(viewport_prediction_t *vpes, float *yaw, float *pitch)
{
  if (vpes == NULL_POINTER || yaw == NULL_POINTER ||
      pitch == NULL_POINTER)
  {
    return RET_FAIL;
  }

  float *yaw_work =
      (float *)malloc(sizeof(float) * vpes->sample_count);
  float *pitch_work =
      (float *)malloc(sizeof(float) * vpes->sample_count);
  int *time_work = (int *)malloc(sizeof(int) * vpes->sample_count);

  if (yaw_work == NULL_POINTER || pitch_work == NULL_POINTER ||
      time_work == NULL_POINTER)
  {
    free(yaw_work);
    free(pitch_work);
    free(time_work);
    return RET_FAIL;
  }

  int samples_to_use = vpes->sample_count;
  int start_idx =
      (vpes->current_index - samples_to_use + vpes->history_size) %
      vpes->history_size;

  for (int i = 0; i < samples_to_use; i++)
  {
    int idx       = (start_idx + i) % vpes->history_size;
    yaw_work[i]   = vpes->yaw_history[idx];
    pitch_work[i] = vpes->pitch_history[idx];
    time_work[i]  = vpes->timestamps[idx];
  }

  adjust_yaw_for_wrapping(yaw_work, samples_to_use);

  float yaw_slope, yaw_intercept;
  float pitch_slope, pitch_intercept;

  RET   yaw_result   = calculate_linear_regression(yaw_work,
                                               time_work,
                                               samples_to_use,
                                               &yaw_slope,
                                               &yaw_intercept);
  RET   pitch_result = calculate_linear_regression(pitch_work,
                                                 time_work,
                                                 samples_to_use,
                                                 &pitch_slope,
                                                 &pitch_intercept);

  free(yaw_work);
  free(pitch_work);
  free(time_work);

  if (yaw_result != RET_SUCCESS || pitch_result != RET_SUCCESS)
  {
    return RET_FAIL;
  }

  int   next_time       = vpes->next_timestamp;
  float predicted_yaw   = yaw_slope * next_time + yaw_intercept;
  float predicted_pitch = pitch_slope * next_time + pitch_intercept;

  *yaw                  = wrap_angle_360(predicted_yaw);
  *pitch                = clamp_pitch(predicted_pitch);

  return RET_SUCCESS;
}