#include "vstream/tile_selection.h"
#include "vstream/viewport_predition.h"
#include <stdlib.h>

// define viewport mxn

#define NO_TILE_ROWS 16
#define NO_TILE_COLS 12

int intersects(float a_min, float a_max, float b_min, float b_max)
{
  return a_min < b_max && a_max > b_min;
}
// a_min for
void define_viewport(float yaw,
                     float pitch,
                     int  *tiles,
                     int  *num_tiles)
{
  *num_tiles    = 0;

  float W_TILE  = W / (float)NO_TILE_COLS;
  float H_TILE  = H / (float)NO_TILE_ROWS;

  float x       = (yaw + 180) / 360 * W;
  float y       = (90 - pitch) / 180 * H;

  float delta_x = (FOV_h / 360) * W;
  float delta_y = (FOV_v / 180) * H;

  // bounding box
  float x_min   = x - delta_x / 2;
  float x_max   = x + delta_x / 2;
  float y_min   = y - delta_y / 2;
  float y_max   = y + delta_y / 2;

  // wrap-around
  if (x_min < 0) // x_min < 0
  {
    float part1_x_min = W + x_min;
    float part1_x_max = W;
    float part2_x_min = 0;
    float part2_x_max = x_max;

    for (COUNT row = 0; row < NO_TILE_ROWS; row++)
    {
      for (COUNT cols = 0; cols < NO_TILE_COLS; cols++)
      {
        float tile_x_min = cols * W_TILE;
        float tile_x_max = (cols + 1) * W_TILE;
        float tile_y_min = row * H_TILE;
        float tile_y_max = (row + 1) * H_TILE;

        if (intersects(
                tile_x_min, tile_x_max, part1_x_min, part1_x_max) &&
            intersects(tile_y_min, tile_y_max, y_min, y_max))
        {
          int tile_id           = row * NO_TILE_COLS + cols;
          tiles[(*num_tiles)++] = tile_id;
        }
        else if (intersects(tile_x_min,
                            tile_x_max,
                            part2_x_min,
                            part2_x_max) &&
                 intersects(tile_y_min, tile_y_max, y_min, y_max))
        {
          int tile_id           = row * NO_TILE_COLS + cols;
          tiles[(*num_tiles)++] = tile_id;
        }
      }
    }
  }
  else if (x_max > W)
  {
    float part1_x_min = 0;
    float part1_x_max = x_max - W;
    float part2_x_min = x_min;
    float part2_x_max = W;

    for (int row = 0; row < NO_TILE_ROWS; ++row)
    {
      for (int col = 0; col < NO_TILE_COLS; ++col)
      {
        float tile_x_min = col * W_TILE;
        float tile_x_max = (col + 1) * W_TILE;
        float tile_y_min = row * H_TILE;
        float tile_y_max = (row + 1) * H_TILE;

        if (intersects(
                tile_x_min, tile_x_max, part1_x_min, part1_x_max) &&
            intersects(tile_y_min, tile_y_max, y_min, y_max))
        {
          int tile_id           = row * NO_TILE_COLS + col;
          tiles[(*num_tiles)++] = tile_id;
        }
        else if (intersects(tile_x_min,
                            tile_x_max,
                            part2_x_min,
                            part2_x_max) &&
                 intersects(tile_y_min, tile_y_max, y_min, y_max))
        {
          int tile_id           = row * NO_TILE_COLS + col;
          tiles[(*num_tiles)++] = tile_id;
        }
      }
    }
  }
  else
  {
    for (int row = 0; row < NO_TILE_ROWS; ++row)
    {
      for (int col = 0; col < NO_TILE_COLS; ++col)
      {
        float tile_x_min = col * W_TILE;
        float tile_x_max = (col + 1) * W_TILE;
        float tile_y_min = row * H_TILE;
        float tile_y_max = (row + 1) * H_TILE;

        if (intersects(tile_x_min, tile_x_max, x_min, x_max) &&
            intersects(tile_y_min, tile_y_max, y_min, y_max))
        {
          int tile_id           = row * NO_TILE_COLS + col;
          tiles[(*num_tiles)++] = tile_id;
        }
      }
    }
  }
}
