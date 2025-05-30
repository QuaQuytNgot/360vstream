#ifndef BUFFER_H
#define BUFFER_H
#include "define.h"
#include <stdlib.h>
// To store the fetched .bin file

typedef struct
{
  char  *data;
  size_t size;
} buffer_t;

RET buffer_init(buffer_t *self);
RET buffer_destroy(buffer_t *self);

#endif