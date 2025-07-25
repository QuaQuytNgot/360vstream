#include <curl/curl.h>
#include <math.h>
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

#define MAX_VIDEO_VERSION 4

char *url           = "https://127.0.0.1:12345";

float yaw_trace[]   = {0.0f, 10.0f, 15.0f};
float pitch_trace[] = {0.0f, 10.0f, 15.0f};
// define more yaw trace and pitch trace
int   size_trace    = sizeof(yaw_trace) / sizeof(yaw_trace[0]);

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
    return -1;
  }

  tile_selection_init(&tse, FIXED_TILE_SELECTION);

  int current_trace_position = min(size_trace, PREDICTION_WINDOW);
  int start_idx = max(0, current_trace_position - PREDICTION_WINDOW);

  for (int i = start_idx; i < current_trace_position; i++)
  {
    add_viewport_sample(&vpes, yaw_trace[i], pitch_trace[i]);
  }

  request_handler_init(&handler, url, 10, 5);
  bw_estimator_init(&bwes, BW_ESTIMATOR_HARMONIC);
  abr_selector_init(&ase, ABR_FOR_NORMAL_BUF);

  float   past_bw[1000]       = {0};
  bw_t   *current_bw_estimate = NULL;
  int     bw_history_count    = 0;
  float   buffer_level        = 0.0f;
  int     num_tiles           = 0;

  clock_t start_time, end_time;
  double  download_time_seconds = 0.0;
  printf("Starting streaming simulation with buffer size: %.2fs\n",
         MAX_BUFFER_SIZE);
  printf("Initial buffer level: %.2fs\n", buffer_level);

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
    int abr_type;

    // call get_dls here to get bw_prediction for the next segment
    // CALL BWES.POST HERE (USING current_bw_estimate)
    // sti buffer here
    if (buffer_level < B_MIN)
    {
      abr_type = ABR_FOR_DANGER_BUF;
      printf("Buffer in DANGER zone (%.2fs < %.2fs)\n",
             buffer_level,
             B_MIN);
    }
    else if (buffer_level >= B_MIN && buffer_level < B_HIGH)
    {
      abr_type = ABR_FOR_NORMAL_BUF;
      printf(
          "Buffer in NORMAL zone (%.2fs - %.2fs)\n", B_MIN, B_HIGH);
    }
    else
    {
      abr_type = ABR_FOR_HIGH_BUF;
      printf("Buffer in HIGH zone (%.2fs+)\n", B_HIGH);
    }

    if (ase.last_quality_default != abr_type)
    {
      abr_selector_init(&ase, abr_type);
    }

    // should declare a weighted parameter here to allocate
    // current_bw_estimate

    // just compute abr for the tile within viewport, else: not yet
    for (int i = 0; i < num_tiles; i++)
    {
      int m_tiles = vp_tiles[i] / NO_OF_COLS;
      int n_tiles = vp_tiles[i] % NO_OF_COLS;
      int tid     = vp_tiles[i];

      chosen_versions[i] =
          ase.choose_bitrate(bwes.dls_es, 3, buffer_level, abr_type);
                            //sth wrongs here
      if (chosen_versions[i] >= handler.version_count)
      {
        chosen_versions[i] = handler.version_count - 1;
      }
    }

    start_time      = clock();

    RET post_result = handler.post(
        &handler, chunk_id, vp_tiles, num_tiles, chosen_versions);

    end_time = clock();
    download_time_seconds =
        ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    if (post_result == RET_FAIL)
    {
      printf("Failed to download tiles for chunk %lld\n", chunk_id);
      free(vp_tiles);
      continue;
    }

    simulate_buffer_increase(&buffer_level);

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

    float playback_time = fmax(download_time_seconds, STEP);
    simulate_buffer_playback(&buffer_level, playback_time);

    if (buffer_level <= 0.0f)
    {
      printf(
          "WARNING: Buffer underrun detected! Video may stall.\n");
    }

    printf("Chunk %lld summary:\n", chunk_id);
    printf("  - Tiles downloaded: %d\n", num_tiles);
    printf("  - Download time: %.3fs\n", download_time_seconds);
    printf("  - Buffer level: %.2fs / %.2fs\n",
           buffer_level,
           MAX_BUFFER_SIZE);
    printf("  - BW estimate: %.2f Mbps\n",
           current_bw_estimate ? *current_bw_estimate / 1000000.0
                               : 0.0);

    handler.reset(&handler);
    free(vp_tiles);
    vp_tiles  = NULL;
    num_tiles = 0;

    usleep(100000);
  }

  request_handler_destroy(&handler);
  bw_estimator_destroy(&bwes);

  free(yaw_history);
  free(pitch_history);
  free(timestamps);
  // call destroy
}
