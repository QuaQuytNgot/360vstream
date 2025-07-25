#ifndef STI_BUFFER_H
#define STI_BUFFER_H

#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void simulate_buffer_increase(float *buffer_level);
void simulate_buffer_playback(float *buffer_level,
                              float  playback_time);

#endif