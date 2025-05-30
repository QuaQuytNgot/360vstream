#ifndef STI_BUFFER_H
#define STI_BUFFER_H

#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void sti_download(float buffer);
void sti_playback(float buffer);
void sti_buffer();

#endif