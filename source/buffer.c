#include "vstream/buffer.h"
#include "vstream/define.h"
#include <stdlib.h>

RET buffer_init(buffer_t *self)
{
  *self = (buffer_t){0};
  return RET_SUCCESS;
}
RET buffer_destroy(buffer_t *self)
{
  if (self->data != NULL_POINTER)
  {
    free(self->data);
  }
  *self = (buffer_t){0};
  return RET_SUCCESS;
}
