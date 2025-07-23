#include <curl/curl.h>
#include <stdio.h>
#include <vstream/abr.h>
#include <vstream/buffer.h>
#include <vstream/bw_prediction.h>
#include <vstream/define.h>
#include <vstream/http.h>
#include <vstream/request_handler.h>
#include <vstream/sti_buffer.h>
#include <vstream/tile_selection.h>
#include <vstream/viewport_predition.h>
#include <math.h>

#define MAX_VIDEO_VERSION 4

char *url = "https://127.0.0.1:12345";

int   main(int argc, char **argv)
{
  request_handler_t     handler          = {0};
  buffer_t             *curr_content_ptr = NULL_POINTER;
  bw_estimator_t        bwes             = {0};
  abr_selector_t        ase              = {0};
  tile_selection_t      tse              = {0};
  viewport_prediction_t vpes;

  int                   history_size = PREDICTION_WINDOW;
  float *yaw_history = (float *)calloc(history_size, sizeof(float));
  float *pitch_history =
      (float *)calloc(history_size, sizeof(float));
  int *timestamps = (int *)calloc(history_size, sizeof(int));

  if (!yaw_history || !pitch_history || !timestamps)
  {
    free(yaw_history);
    free(pitch_history);
    free(timestamps);
    return -1;
  }

  RET init_result =
      viewport_prediction_init(&vpes,
                               yaw_history,
                               pitch_history,
                               timestamps,
                               0,
                               0,
                               0,
                               history_size,
                               VIEWPORT_ESTIMATOR_LEGR);

  if (init_result != RET_SUCCESS)
  {
    free(yaw_history);
    free(pitch_history);
    free(timestamps);
    return;
  }

  int current_trace_position = min(size_trace, PREDICTION_WINDOW);
  int start_idx = max(0, current_trace_position - PREDICTION_WINDOW);

  for (int i = start_idx; i < current_trace_position; i++)
  {
    add_viewport_sample(&vpes, yaw_trace[i], pitch_trace[i]);
  }

  request_handler_init(&handler, url, 10, 5);
  bw_estimator_init(&bwes, BW_ESTIMATOR_HARMONIC);
  abr_selector_init(&ase, ABR_FOR_NORMAL_BUF);

  float past_bw[1000]       = {0};
  bw_t *current_bw_estimate = NULL;
  int   bw_history_count    = 0;
  float buffer_level        = 0.0f;
  int   num_tiles           = 0;

  for (COUNT chunk_id = 0; chunk_id < handler.seg_count; chunk_id++)
  {
    // 1. xac dinh vp
    int *vp_tiles =
        (int *)malloc(sizeof(int) * NO_OF_ROWS * NO_OF_COLS);
    if (vp_tiles == NULL)
    {
      perror("Failed to allocate memory for vp_tiles");
      exit(EXIT_FAILURE);
    }
    num_tiles = 0;

    float predicted_yaw, predicted_pitch;

    if ((current_trace_position + chunk_id) < size_trace)
    {
      add_viewport_sample(
          &vpes,
          yaw_trace[current_trace_position + chunk_id],
          pitch_trace[current_trace_position + chunk_id]);
    }
    // call vp prediction function here (that return yaw, pitch)

    RET prediction_result =
        vpes.vpes_post(&vpes, &predicted_yaw, &predicted_pitch);

    if (prediction_result == RET_SUCCESS)
    {
      printf("Predicted Yaw: %.2f, Predicted Pitch: %.2f\n",
             predicted_yaw,
             predicted_pitch);
    }

    tse.select_viewport(
        predicted_yaw, predicted_pitch, &vp_tiles, &num_tiles);
    // them ham dynamic tiling stratgy to change vp_tiles and
    // num_tiles here dynamic_tiling_strategy(&vp_tiles, &num_tiles,
    // buffer_level, current_bw_estimate);

    if (num_tiles <= 0)
    {
      printf("No tiles selected for chunk %lld\n", chunk_id);
      free(vp_tiles);
      continue;
    }

    //  2. tinh abr
    int chosen_versions[num_tiles];

    // call get_dls here to get bw_prediction for the next segment
    // CALL BWES.POST HERE (USING current_bw_estimate)
    // sti buffer here
    int abr_type;
    if (buffer_level < B_MIN)
    {
      abr_type = ABR_FOR_DANGER_BUF;
    }
    else if (buffer_level >= B_MIN && buffer_level < B_HIGH)
    {
      abr_type = ABR_FOR_NORMAL_BUF;
    }
    else
    {
      abr_type = ABR_FOR_HIGH_BUF;
    }

    // Reinitialize ABR selector if needed
    if (ase.last_quality_default != abr_type)
    {
      abr_selector_init(&ase, abr_type);
    }

    // should declare a weighted parameter here to allocate
    // current_bw_estimate

    for (int i = 0; i < num_tiles; i++)
    {
      int m_tiles = vp_tiles[i] / NO_OF_COLS;
      int n_tiles = vp_tiles[i] % NO_OF_COLS;
      int tid     = vp_tiles[i];

      chosen_versions[i] =
          ase.choose_bitrate(bwes.dls_es, 3, buffer_level, abr_type);

      if (chosen_versions[i] >= handler.version_count)
      {
        chosen_versions[i] = handler.version_count - 1;
      }
    }

    //  3. goi post tai version tuong ung
    RET post_result = handler.post(
        &handler, chunk_id, vp_tiles, num_tiles, chosen_versions);
    if (post_result == RET_FAIL)
    {
      printf("Failed to download tiles for chunk %lld\n", chunk_id);
      free(vp_tiles);
      continue;
    }

    // can compute total bw in here
    bw_t tile_speeds[num_tiles];
    int  valid_speeds = 0;

    for (int i = 0; i < num_tiles; i++)
    {
      int  tile_id = vp_tiles[i];
      bw_t speed =
          handler.dls[tile_id][chunk_id % handler.seg_count];

      if (speed > 0)
      {
        tile_speeds[valid_speeds++] = speed;
      }
    }

    if (valid_speeds > 0)
    {
      bwes.post(&bwes, tile_speeds, valid_speeds);
      bwes.get(&bwes, &current_bw_estimate);
    }

    // update param here
    float download_time = 0.0f;
    if (current_bw_estimate && *current_bw_estimate > 0)
    {
      for (int i = 0; i < num_tiles; i++)
      {
        int   version    = chosen_versions[i];
        float chunk_size = VIDEO_BIT_RATE[version];
        download_time += chunk_size / *current_bw_estimate;
      }

      // Update buffer level (assuming segment duration is 1 second)
      float segment_duration = 1.0f;
      buffer_level = buffer_level + segment_duration - download_time;
      if (buffer_level < 0)
        buffer_level = 0;
    }

    printf(
        "Chunk %lld: Buffer level: %.2f, BW estimate: %.2f Mbps\n",
        chunk_id,
        buffer_level,
        current_bw_estimate ? *current_bw_estimate / 1000000.0
                            : 0.0);

    handler.reset(&handler);
    free(vp_tiles);
    vp_tiles  = NULL;
    num_tiles = 0;
  }
  request_handler_destroy(&handler);
  bw_estimator_destroy(&bwes);
  
  free(yaw_history);
  free(pitch_history);
  free(timestamps);
  // call destroy
}
