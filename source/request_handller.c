#include "vstream/buffer.h"
#include "vstream/bw_prediction.h"
#include "vstream/define.h"
#include "vstream/request_handler.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

COUNT tile_count = NO_OF_ROWS * NO_OF_COLS;

RET   request_handler_init(request_handler_t *self,
                           const char        *ser_adrr,
                           COUNT              seg_count,
                           COUNT              version_count)
{
  tile_count          = NO_OF_COLS * NO_OF_ROWS;
  self->tile_count    = tile_count;
  self->seg_count     = seg_count;
  self->ser_addr      = strdup(ser_adrr);
  self->version_count = version_count;

  self->curr_seg      = malloc(tile_count * sizeof(buffer_t *));
  if (self->curr_seg == NULL_POINTER)
  {
    return RET_FAIL;
  }
  for (COUNT i = 0; i < tile_count; i++)
  {
    self->curr_seg[i] =
        malloc(self->version_count * sizeof(buffer_t));
    if (self->curr_seg[i] == NULL_POINTER)
    {
      for (COUNT j = 0; j < i; j++)
      {
        free(self->curr_seg[j]);
      }
      free(self->curr_seg);
      return RET_FAIL;
    }
    for (COUNT s = 0; s < self->version_count; s++)
    {
      buffer_init(&self->curr_seg[i][s]);
    }
  }

  self->dls = malloc(tile_count * sizeof(bw_t *));
  if (self->dls == NULL_POINTER)
  {
    return RET_FAIL;
  }

  for (COUNT i = 0; i < tile_count; i++)
  {
    self->dls[i] = malloc(self->seg_count * sizeof(bw_t));
    if (self->dls[i] == NULL_POINTER)
    {
      return RET_FAIL;
    }
    for (COUNT j = 0; j < self->seg_count; j++)
    {
      self->dls[i][j] = 0;
    }
  }

  self->post         = request_handler_post;
  self->reset        = request_handler_reset;
  self->get_dl_speed = request_handler_get_dls;

  return RET_SUCCESS;
}
// init cac tham so trong struct, init mang curr_seg de luu file
// .bin, init mang dls de luu past_bw

RET request_handler_destroy(request_handler_t *self)
{
  if (self == NULL)
  {
    return RET_FAIL;
  }

  if (self->ser_addr != NULL)
  {
    free(self->ser_addr);
    self->ser_addr = NULL;
  }

  if (self->curr_seg != NULL_POINTER)
  {
    for (COUNT i = 0; i < self->tile_count; i++)
    {
      for (COUNT s = 0; s < self->version_count; s++)
      {
        buffer_destroy(&self->curr_seg[i][s]);
      }
      free(self->curr_seg[i]);
      self->curr_seg[i] = NULL_POINTER;
    }
  }
  free(self->curr_seg);
  self->curr_seg = NULL_POINTER;
  // free curr_seg, dls

  if (self->dls != NULL_POINTER)
  {
    for (COUNT i = 0; i < self->tile_count; i++)
    {
      if (self->dls[i] != NULL_POINTER)
      {
        free(self->dls[i]);
        self->dls[i] = NULL_POINTER;
      }
    }
    free(self->dls);
    self->dls = NULL_POINTER;
  }

  // set pointer function to NULL_POINTER
  self->post         = NULL_POINTER;
  self->reset        = NULL_POINTER;
  self->get_dl_speed = NULL_POINTER;

  return RET_SUCCESS;
}

RET request_handler_post(request_handler_t *self,
                         COUNT              chunk_id,
                         int               *vp_tiles,
                         int                num_vp_tiles,
                         int               *chosen_versions)
{
  if (self == NULL)
  {
    return RET_FAIL;
  }

  char url_buffer[512];

  for (int i = 0; i < num_vp_tiles; i++)
  {
    int tile_id = vp_tiles[i];
    int version = chosen_versions[i];

    if (tile_id >= self->tile_count ||
        version >= self->version_count)
    {
      continue;
    }

    snprintf(
        url_buffer,
        sizeof(url_buffer),
        "%s/beach_%lld/erp_8x6/tile_yuv/tile_%d_%d_480x360_QP%d.bin",
        self->ser_addr,
        chunk_id,
        tile_id / NO_OF_COLS,
        tile_id % NO_OF_COLS,
        tile_version_to_num(version));
    // should change erp_%dx%d for adaptive tiling

    // printf("%s\n", url);

    RET r = http_get_to_buffer(
        url_buffer,
        STREAM_HTTP_3_0,
        &self->curr_seg[tile_id][version],
        &self->dls[tile_id][chunk_id % self->seg_count]);

    if (r == RET_FAIL)
    {
      printf("Failed to download viewport tile %d, version %d\n",
             tile_id,
             version);
      continue;
    }
  }

  // download all non-viewport tiles at lowest quality (version 0)
  for (COUNT tile_id = 0; tile_id < self->tile_count; tile_id++)
  {
    // Check if this tile is in viewport
    bool is_viewport_tile = false;
    for (int i = 0; i < num_vp_tiles; i++)
    {
      if (vp_tiles[i] == tile_id)
      {
        is_viewport_tile = true;
        break;
      }
    }

    // Download non-viewport tiles at version 0
    if (!is_viewport_tile)
    {
      snprintf(url_buffer,
               sizeof(url_buffer),
               "%s/beach_%lld/erp_8x6/tile_yuv/"
               "tile_%d_%d_480x360_QP38.bin",
               self->ser_addr,
               chunk_id,
               tile_id / NO_OF_COLS,
               tile_id % NO_OF_COLS);

      // printf("%s\n", url);

      http_get_to_buffer(
          url_buffer,
          STREAM_HTTP_3_0,
          &self->curr_seg[tile_id][0],
          &self->dls[tile_id][chunk_id % self->seg_count]);
    }
  }

  return RET_SUCCESS;
}

RET request_handler_reset(request_handler_t *self)
{
  if (self == NULL)
  {
    return RET_FAIL;
  }

  for (COUNT i = 0; i < self->tile_count; i++)
  {
    for (COUNT j = 0; j < self->version_count; j++)
    {
      buffer_destroy(&self->curr_seg[i][j]);
      buffer_init(&self->curr_seg[i][j]);
    }
  }

  return RET_SUCCESS;
}

RET request_handler_get_dls(request_handler_t *self, bw_t **real_bw)
{
  if (self->dls == NULL)
  {
    return RET_FAIL;
  }
  *real_bw = self->dls;
}
