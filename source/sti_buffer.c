#include "vstream/sti_buffer.h"
#include "vstream/abr.h"
#include "vstream/define.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define segment_arrive 1

void simulate_buffer_increase(float *buffer_level)
{
  *buffer_level += SEGMENT_DURATION;
  if (*buffer_level > MAX_BUFFER_SIZE)
  {
    *buffer_level = MAX_BUFFER_SIZE;
  }
  printf("Buffer increased by %.2fs. Current buffer: %.2fs\n",
         SEGMENT_DURATION,
         *buffer_level);
}

void simulate_buffer_playback(float *buffer_level,
                              float  playback_time)
{
  if (*buffer_level > 0.0f)
  {
    *buffer_level -= playback_time;
    if (*buffer_level < 0.0f)
    {
      *buffer_level = 0.0f;
    }
  }
  printf("Buffer decreased by %.2fs. Current buffer: %.2fs\n",
         playback_time,
         *buffer_level);
}
