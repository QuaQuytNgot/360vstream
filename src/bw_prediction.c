#include "bw_prediction.h"
#include "define.h"
#include <math.h>

// double harmonic_mean(double arr[], int len)
// {
//     int sum = 0;
//     for (int i = 0; i < len; i++)
//     {
//         sum += 1/arr[i];
//     }
//     int ans =  len/sum;
//     return ans;
// }

RET bw_estimator_init(bw_estimator_t *self, int type)
{
    *self = (bw_estimator_t){0};
    self->dls_es = BW_DEFAULT;

    switch (type)
    {
    case BW_ESTIMATOR_HARMONIC:
        self->post = bw_estimator_post_harmonic_mean;
        self->get = bw_estimator_get_harmonic_mean;
        break;

    default:
        self->post = bw_estimator_post_harmonic_mean;
        self->get = bw_estimator_get_harmonic_mean;
        break;
    }
}

RET bw_estimator_destroy(bw_estimator_t *self)
{
    *self = (bw_estimator_t){0};
    self->dls_es = BW_DEFAULT;
    return RET_SUCCESS;
}

RET bw_estimator_post_harmonic_mean(bw_estimator_t *self,
                                    bw_t *dls_arr,
                                    size_t dls_arr_count)
{
    int count = 0;
    float sum = 0.0f;
    self->dls_es = BW_DEFAULT;

    if (dls_arr_count == 0)
    {
        return RET_FAIL;
    }
    for (size_t i = 0; i < dls_arr_count; i++)
    {
        if (dls_arr[i] >= 0)
        {
            ++count;
            sum += 1.0F / (float)dls_arr[i];
        }
    }
    self->dls_es = (bw_t)((float)count / sum);

    return RET_SUCCESS;
}

RET bw_estimator_get_harmonic_mean(bw_estimator_t *self,
                                   bw_t *dls_es)
{
    if (self->dls_es == BW_DEFAULT)
    {
        return RET_FAIL;
    }

    *dls_es = self->dls_es;
    return RET_SUCCESS;
}