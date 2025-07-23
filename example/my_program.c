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

#define current_yaw       1.0
#define current_pitch     1.0
#define MAX_VIDEO_VERSION 4

char *url = "https://127.0.0.1:12345";

int   main(int argc, char **argv)
{
  request_handler_t handler          = {0};
  buffer_t         *curr_content_ptr = NULL_POINTER;
  bw_estimator_t    bwes             = {0};
  abr_selector_t    ase              = {0};
  tile_selection_t  tse              = {0};

  tile_selection_init(&tse, VIEWPORT_ESTIMATOR_SPHERICAL_WALK);
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
    int num_tiles = 0;

    // call vp prediction function here (that return yaw, pitch)

    tse.select_viewport(
        current_yaw, current_pitch, &vp_tiles, &num_tiles);
    // them ham dynamic tiling stratgy to change vp_tiles and
    // num_tiles here dynamic_tiling_strategy(&vp_tiles, &num_tiles,
    // buffer_level, current_bw_estimate);

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
    for (int i = 0; i < num_tiles; i++)
    {
      int   version    = chosen_versions[i];
      float chunk_size = VIDEO_BIT_RATE[version];
      download_time += chunk_size / *current_bw_estimate;
    }
    // update buffer_level here

    // update last quality for next iteration here

    //  6. Process downloaded content (optional)
    printf(
        "Chunk %lld: Buffer level: %.2f, BW estimate: %.2f Mbps\n",
        chunk_id,
        buffer_level,
        *current_bw_estimate / 1000000.0);
    //  Here can access the downloaded tile data via:
    //  handler.curr_seg[tile_id][version] for each downloaded tile
    handler.reset(&handler);

    // should write a function for free vp_tiles
    free(vp_tiles);
    vp_tiles = NULL;
    num_tiles = 0;
  }
  request_handler_destroy(&handler);
  bw_estimator_destroy(&bwes);
  // call destroy
}
