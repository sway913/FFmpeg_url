#include <fcntl.h>
#if HAVE_IO_H
#include <io.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <stdlib.h>
#include "WasmFetch.h"

#define WASM_HTTP_TAG "[wasmhttp]"
#define MAX_FLIENAME_LEN 512

typedef struct WasmHttpContext
{
    const AVClass *class;
    char filename[MAX_FLIENAME_LEN];
    int64_t filesize;
    int64_t pos;
} WasmHttpContext;

static const AVOption wasm_options[] = {
    {NULL}};

static const AVClass wasmhttp_class = {
    .class_name = "wasmhttp",
    .item_name = av_default_item_name,
    .option = wasm_options,
    .version = LIBAVUTIL_VERSION_INT,
};

static int wasmhttp_read(URLContext *h, unsigned char *buf, int size)
{
    WasmHttpContext *c = h->priv_data;
    if (c->pos == c->filesize)
    {
        av_log(NULL, AV_LOG_INFO, "%s eof %s\n", WASM_HTTP_TAG, c->filename);
        return AVERROR_EOF;
    }

    int readSize = 0;
    int64_t nextPos = c->pos + size - 1;
    if (c->pos + size >= c->filesize)
    {
        nextPos = c->filesize;
        av_log(NULL, AV_LOG_INFO, "%s read end %s\n", WASM_HTTP_TAG, c->filename);
    }
    av_log(NULL, AV_LOG_DEBUG, "wasmhttp read (%lld-%lld)\n", c->pos, nextPos);
    readSize = wasm_fech(c->filename + 11, c->pos, nextPos, buf);
    if (readSize <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "%s fech failed %s.\n", WASM_HTTP_TAG, c->filename);
        return AVERROR_EXTERNAL;
    }

    c->pos = c->pos + readSize;
    return readSize;
}

static int wasmhttp_write(URLContext *h, const unsigned char *buf, int size)
{
    return 0;
}

static int wasmhttp_delete(URLContext *h)
{
    return 0;
}

static int wasmhttp_move(URLContext *h_src, URLContext *h_dst)
{
    return 0;
}

static int wasmhttp_open(URLContext *h, const char *filename, int flags)
{
    WasmHttpContext *c = h->priv_data;

    if (filename == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "%s open failed\n", WASM_HTTP_TAG);
        return AVERROR_EXTERNAL;
    }

    memcpy(c->filename, filename, strlen(filename) + 1);
    c->filesize = wasm_fetch_get_filesize(c->filename + 11);
    if (c->filesize <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "%s can not get filesize \n" WASM_HTTP_TAG);
        return AVERROR_EXTERNAL;
    }
    h->max_packet_size = 2 * 1024 * 1024;

    av_log(NULL, AV_LOG_DEBUG, "%s open success filesize = %lld filename = %s \n", WASM_HTTP_TAG, c->filesize, c->filename);
    return 0;
}

static int64_t wasmhttp_seek(URLContext *h, int64_t pos, int whence)
{
    WasmHttpContext *c = h->priv_data;
    if (whence == AVSEEK_SIZE)
    {
        long long file_size = c->filesize;
        av_log(NULL, AV_LOG_INFO, "%s AVSEEK_SIZE %lld \n", WASM_HTTP_TAG, c->filesize);
        return file_size;
    }
    else if (whence == 0)
    {
        c->pos = pos;
    }
    else if (whence == 1)
    {
        c->pos += pos;
    }
    else
    {
        av_log(NULL, AV_LOG_WARNING, "%s wasmhttp seek not support %d \n", WASM_HTTP_TAG, whence);
    }
    av_log(NULL, AV_LOG_DEBUG, "%s wasmhttp seek pos = %lld,c->pos = %lld,whence = %d \n", pos, c->pos, whence);
    if (c->pos > c->filesize)
    {
        av_log(NULL, AV_LOG_ERROR, "%s wasmhttp seek failed \n", WASM_HTTP_TAG);
        return AVERROR_EXTERNAL;
    }

    return c->pos;
}

static int wasmhttp_close(URLContext *h)
{
    WasmHttpContext *c = h->priv_data;
    av_log(NULL, AV_LOG_DEBUG, "%s close %s\n", WASM_HTTP_TAG, c->filename);
    return 0;
}

const URLProtocol ff_wasm_http_protocol = {
    .name = "wasmhttp",
    .url_open = wasmhttp_open,
    .url_read = wasmhttp_read,
    .url_write = wasmhttp_write,
    .url_seek = wasmhttp_seek,
    .url_close = wasmhttp_close,
    .url_delete = wasmhttp_delete,
    .url_move = wasmhttp_move,
    .priv_data_size = sizeof(WasmHttpContext),
    .priv_data_class = &wasmhttp_class,
    .default_whitelist = "wasmhttp"
    };
