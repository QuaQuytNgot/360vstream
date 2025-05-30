#ifndef BW_PREDICTION_H
#define BW_PREDICTION_H
#include "define.h"
#include <stddef.h>

typedef struct bw_estimator_t bw_estimator_t;

struct bw_estimator_t
{
    bw_t dls_es;
    
    RET (*post)(bw_estimator_t *, bw_t *, size_t);
    RET (*get)(bw_estimator_t *, bw_t *);
};

RET bw_estimator_init(bw_estimator_t *self, int type);

RET bw_estimator_destroy(bw_estimator_t *self);

RET bw_estimator_post_harmonic_mean(bw_estimator_t *self,
                                bw_t *dls_arr,
                                size_t dls_arr_count);
RET bw_estimator_get_harmonic_mean(bw_estimator_t *self,
                               bw_t *dls_es);

// post: ghi, cap nhat gia tri
// get: truy xuat gia tri tu post
#endif