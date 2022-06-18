#ifndef __WASM_FETCH__
#define __WASM_FETCH__

#include <stdio.h>
#include "stdint.h"
#include "libavutil/avstring.h"
#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int64_t wasm_fetch_get_filesize(char *filename);
    int wasm_fech(char *filename, int64_t pos, int64_t to, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif