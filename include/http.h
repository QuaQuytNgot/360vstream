#ifndef HTTP_H
#define HTTP_H
#include "define.h"
#include "buffer.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <stddef.h>

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

RET http_get_to_buffer(const char *url,
                       HTTP_VERSION ver,
                       buffer_t *data,
                       bw_t *dls);

int tile_version_to_num(int version);

#endif