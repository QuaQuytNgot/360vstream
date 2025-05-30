#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H
#include "abr.h"
#include "define.h"
#include "http.h"
#include "tile_selection.h"

typedef struct request_handler_t request_handler_t;

struct request_handler_t
{
  char      *ser_addr;
  COUNT      seg_count; // no of segment
  COUNT      tile_count;
  COUNT      version_count;

  buffer_t **curr_seg; // curr_seg[tile_id][layer]
  bw_t     **dls;
  // tile_selection_t *vsel;
  // abr_selector_t *abr;

  RET (*post)(request_handler_t *, COUNT, int *, int, int *);
  RET (*reset)(request_handler_t *);
  RET (*get_dl_speed)(request_handler_t *, bw_t *);
};

RET request_handler_post(request_handler_t *self,
                         COUNT              chunk_id,
                         int               *vp_tiles,
                         int                num_vp_tiles,
                         int               *chosen_version);
RET request_handler_destroy(request_handler_t *self);
RET request_handler_reset(request_handler_t *self);
RET request_handler_init(request_handler_t *self,
                         const char        *ser_adrr,
                         COUNT              seg_count,
                         COUNT              version_count);
RET request_handler_get_dls(request_handler_t *, bw_t *);
#endif