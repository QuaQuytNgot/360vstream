#include "abr.h"
#include <stdlib.h>
#include "bw_prediction.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

double VIDEO_BIT_RATE[5] = {0, 1, 2, 3, 4};

RET abr_selector_init(abr_selector_t *abr_selector, int type)
{
    switch (type)
    {
    case ABR_FOR_NORMAL_BUF:
        abr_selector->choose_bitrate = abr_for_normal_buf;
        abr_selector->last_quality_default = NO_VIDEO_VERSION_NORMAL;
        break;
    case ABR_FOR_DANGER_BUF:
        abr_selector->choose_bitrate = abr_for_danger_buf;
        abr_selector->choose_bitrate = NO_VIDEO_VERSION_DANGER;
        break;
    case ABR_FOR_HIGH_BUF:
        abr_selector->choose_bitrate = abr_for_high_buf;
        abr_selector->choose_bitrate = NO_VIDEO_VERSION_HIGH;
        break;
    default:
        return RET_FAIL;
        break;
    }
    return RET_SUCCESS;
}

int abr_for_normal_buf(float bw_prediction,
                       int P, float buffer_size,
                       int last_quality)
{
    int total_combos = pow(NO_VIDEO_VERSION_NORMAL, P);
    int CHUNK_COMBO_OPTIONS[total_combos][P];
    for (int i = 0; i < total_combos; i++)
    {
        int reversed_i = total_combos - i - 1;
        int temp = reversed_i;
        for (int j = 0; j < P; j++)
        {
            CHUNK_COMBO_OPTIONS[i][j] = temp % NO_VIDEO_VERSION_NORMAL;
            temp /= NO_VIDEO_VERSION_NORMAL;
        }
    }
    // done initial CHUNK_COMBO_OPTIONS[i][j]

    int best_combo[5] = {};
    float max_reward = -10000.0;

    for (int i = 0; i < total_combos; i++)
    {
        bool quality_diff_too_big = false;
        int combo_arr[P];
        memset(combo_arr, 0, sizeof(combo_arr));

        for (int j = 0; j < P; j++)
        {
            combo_arr[j] = CHUNK_COMBO_OPTIONS[i][j];
        }

        // check for quality_diff_too_big to limit the computation
        for (int j = 0; j < P - 1; j++)
        {
            if (abs(combo_arr[j] - combo_arr[j + 1]) > 3)
            {
                quality_diff_too_big = true;
                break;
            }
        }
        if (abs(combo_arr[0] - last_quality) > 3)
            quality_diff_too_big = true;
        if (quality_diff_too_big)
            continue;

        // start computing bitrate_sum, rebuffer, smooth
        float curr_rebuff = 0.0f;
        float bitrate_sum = 0.0f;
        float smoothness = 0.0f;
        float curr_buffer = buffer_size;

        for (int j = 0; j < P; j++) // duyet combo
        {
            bitrate_sum += VIDEO_BIT_RATE[combo_arr[j]];
            // done bitrate sum

            int chunk_quality = CHUNK_COMBO_OPTIONS[i][j];
            float chunk_size = VIDEO_BIT_RATE[combo_arr[j]];
            float download_time = chunk_size / bw_prediction;
            if (curr_buffer < download_time)
            {
                curr_rebuff += (download_time - curr_buffer);
                curr_buffer = 0;
            }
            else
                curr_buffer -= download_time;
            // done rebuffer

            smoothness += fabs(VIDEO_BIT_RATE[chunk_quality] - VIDEO_BIT_RATE[last_quality]);
            last_quality = chunk_quality;
            // done smoothnesslast_quality
        }

        float reward;
        reward = alpha * (bitrate_sum / 1000.0) - beta * (curr_rebuff / 1000.0) - theta * (smoothness / 1000.0);

        if (reward > max_reward)
        {
            for (int k = 0; k < P; k++)
                best_combo[k] = CHUNK_COMBO_OPTIONS[i][k];
            max_reward = reward;
        }
    }
    return best_combo[0];
}

int abr_for_danger_buf(float bw_prediction,
                       int P, float buffer_size,
                       int last_quality)
{
    int total_combos = pow(NO_VIDEO_VERSION_DANGER, P);
    int CHUNK_COMBO_OPTIONS[total_combos][P];
    for (int i = 0; i < total_combos; i++)
    {
        int temp = i;
        for (int j = 0; j < P; j++)
        {
            CHUNK_COMBO_OPTIONS[i][j] = temp % 3;
            temp /= 3;
        }
    }
    // done initial CHUNK_COMBO_OPTIONS[i][j]

    int best_combo[5] = {};
    float max_reward = -10000.0;

    for (int i = 0; i < total_combos; i++)
    {
        int combo_arr[P];
        memset(combo_arr, 0, sizeof(combo_arr));

        for (int j = 0; j < P; j++)
        {
            combo_arr[j] = CHUNK_COMBO_OPTIONS[i][j];
        }

        float curr_rebuff = 0.0f;
        float bitrate_sum = 0.0f;
        float smoothness = 0.0f;
        float curr_buffer = buffer_size;

        for (int j = 0; j < P; j++)
        {
            bitrate_sum += VIDEO_BIT_RATE[combo_arr[j]];
            // done bitrate sum

            int chunk_quality = CHUNK_COMBO_OPTIONS[i][j];
            float chunk_size = VIDEO_BIT_RATE[combo_arr[j]];
            float download_time = chunk_size / bw_prediction;
            if (curr_buffer < download_time)
            {
                curr_rebuff += (download_time - curr_buffer);
                curr_buffer = 0;
            }
            else
                curr_buffer -= download_time;
            // done rebuffer

            smoothness += fabs(VIDEO_BIT_RATE[chunk_quality] - VIDEO_BIT_RATE[last_quality]);
            last_quality = chunk_quality;
            // done smoothnesslast_quality
        }

        float reward;
        reward = alpha * (bitrate_sum / 1000.0) - beta * (curr_rebuff / 1000.0) - theta * (smoothness / 1000.0);

        if (reward > max_reward)
        {
            for (int k = 0; k < P; k++)
                best_combo[k] = CHUNK_COMBO_OPTIONS[i][k];
            max_reward = reward;
        }
    }
    return best_combo[0];
}

abr_for_high_buf(float bw_prediction,
                 int P, float buffer_size,
                 int last_quality)
{
    int total_combos = pow(NO_VIDEO_VERSION_HIGH, P);
    int CHUNK_COMBO_OPTIONS[total_combos][P];
    for (int i = 0; i < total_combos; i++)
    {
        int reversed_i = total_combos - i - 1;
        int temp = reversed_i;
        for (int j = 0; j < P; j++)
        {
            CHUNK_COMBO_OPTIONS[i][j] = temp % NO_VIDEO_VERSION_HIGH;
            temp /= NO_VIDEO_VERSION_HIGH;
        }
    }
    // done initial CHUNK_COMBO_OPTIONS[i][j]

    int best_combo[5] = {};
    float max_reward = -10000.0;

    for (int i = 0; i < total_combos; i++)
    {
        bool quality_diff_too_big = false;
        int combo_arr[P];
        memset(combo_arr, 0, sizeof(combo_arr));

        for (int j = 0; j < P; j++)
        {
            combo_arr[j] = CHUNK_COMBO_OPTIONS[i][j];
        }

        // check for quality_diff_too_big to limit the computation
        for (int j = 0; j < P - 1; j++)
        {
            if (abs(combo_arr[j] - combo_arr[j + 1]) > 3)
            {
                quality_diff_too_big = true;
                break;
            }
        }
        if (abs(combo_arr[0] - last_quality) > 3)
            quality_diff_too_big = true;
        if (quality_diff_too_big)
            continue;

        // start computing bitrate_sum, rebuffer, smooth
        float curr_rebuff = 0.0f;
        float bitrate_sum = 0.0f;
        float smoothness = 0.0f;
        float curr_buffer = buffer_size;

        for (int j = 0; j < P; j++) // duyet combo
        {
            bitrate_sum += VIDEO_BIT_RATE[combo_arr[j]];
            // done bitrate sum

            int chunk_quality = CHUNK_COMBO_OPTIONS[i][j];
            float chunk_size = VIDEO_BIT_RATE[combo_arr[j]];
            float download_time = chunk_size / bw_prediction;
            if (curr_buffer < download_time)
            {
                curr_rebuff += (download_time - curr_buffer);
                curr_buffer = 0;
            }
            else
                curr_buffer -= download_time;
            // done rebuffer

            smoothness += fabs(VIDEO_BIT_RATE[chunk_quality] - VIDEO_BIT_RATE[last_quality]);
            last_quality = chunk_quality;
            // done smoothnesslast_quality
        }

        float reward;
        reward = alpha * (bitrate_sum / 1000.0) - beta * (curr_rebuff / 1000.0) - theta * (smoothness / 1000.0);

        if (reward > max_reward)
        {
            for (int k = 0; k < P; k++)
                best_combo[k] = CHUNK_COMBO_OPTIONS[i][k];
            max_reward = reward;
        }
    }
    return best_combo[0];
}
