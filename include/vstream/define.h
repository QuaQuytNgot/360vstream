#ifndef DEFINE_H
#define DEFINE_H

#include "define.h"
#include <math.h>
#include <stdint.h>

#define BW                long int // bytes/s
#define BYTE              unsigned int
#define RET               unsigned int
#define RET_FAIL          0
#define RET_SUCCESS       1
#define BW_DEFAULT        (-1)
#define HISTORY_SIZE      100
#define PREDICTION_WINDOW 10

typedef uint64_t bw_t;
typedef uint64_t count_t;

#define MAX_BUFFER_SIZE  4.0f
#define STEP             0.01f
#define SEGMENT_DURATION 1.0f
#define B_MIN            1.0f
#define B_NORMAL         2.0f
#define B_HIGH           3.0f

typedef enum
{
  BW_ESTIMATOR_HARMONIC = 1
} BW_ESTIMATOR;

typedef enum
{
  STREAM_HTTP_2_0 = 1,
  STREAM_HTTP_3_0
} HTTP_VERSION;

typedef enum
{
  VIEWPORT_ESTIMATOR_LEGR = 1
} VIEWPORT_ESTIMATOR;

typedef enum
{
  DYNAMIC_TILE_SELECTION = 1,
  FIXED_TILE_SELECTION
} TILE_SELECTION;

typedef enum
{
  ABR_FOR_NORMAL_BUF = 1,
  ABR_FOR_DANGER_BUF,
  ABR_FOR_HIGH_BUF
} ABR_SCHEME;

// for abr alg
#define NO_VIDEO_VERSION_NORMAL 5
#define NO_VIDEO_VERSION_DANGER 3
#define NO_VIDEO_VERSION_HIGH   4
#define alpha                   1
#define beta                    1.85
#define theta                   1

// for tile selection strategy
#define roll                    0
#define W                       3840
#define H                       1920
#define FOV_h                   90
#define FOV_v                   90

#define NO_OF_ROWS              6
#define NO_OF_COLS              4

typedef unsigned long long COUNT;

#define NULL_POINTER ((void *)0)

float yaw_trace[]   = {0.0f, 10.0f, 15.0f};
float pitch_trace[] = {0.0f, 10.0f, 15.0f};
// define more yaw trace and pitch trace
int   size_trace    = sizeof(yaw_trace) / sizeof(yaw_trace[0]);
#endif