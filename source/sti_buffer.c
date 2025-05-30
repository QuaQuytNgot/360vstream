#include "vstream/sti_buffer.h"
#include "vstream/abr.h"
#include "vstream/define.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define segment_arrive 1

void sti_download(float buffer)
{
  if (segment_arrive)
  {
    if (buffer < B_MIN)
    {
      // download version 0, 1
      buffer += SEGMENT_DURATION;
    }
    else if (buffer >= B_MIN && buffer < B_HIGH)
    {
      // downloading enhancement segment 0 1 2 3 4
      // continue the iteration if 3 segment prediction is 4 1 ..
      // (1st - 2nd >= 3)
    }
    else
    { // buffer > B_HIGH: downloading enhancement segment 2 3 4
    }
  }
}

void sti_playback(float buffer)
{
  if (buffer > 0.0f)
  {
    buffer -= STEP;
  }
  // only -= STEP bc the sti_download() will guarantee buff will be
  // not empty as well as over
}

void sti_buffer()
{
  float buffer = 0.0f;

  srand(time(NULL));

  while (1)
  {
    sti_download(buffer);
    sti_playback(buffer);
    usleep(10000);
  }
}
