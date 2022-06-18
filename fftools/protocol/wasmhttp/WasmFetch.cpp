#include "WasmFetch.h"
#include <emscripten/fetch.h>
#include <emscripten/val.h>
#include <string>

int64_t wasm_fetch_get_filesize(char *filename)
{
    int64_t file_size = 0;
    const char *headers[] = {"Range", "bytes=0-0", NULL};

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    attr.requestHeaders = headers;
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, filename);
    if (fetch->status == 200 || fetch->status == 206)
    {
        size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch) + 1;
        char *headerString = new char[headersLengthBytes];
        emscripten_fetch_get_response_headers(fetch, headerString, headersLengthBytes);

        av_log(NULL, AV_LOG_DEBUG, "fetch:get file size header %s,name = %s ,status =%d .\n", headerString, filename, fetch->status);
        char **responseHeaders = emscripten_fetch_unpack_response_headers(headerString);
        delete[] headerString;

        int numHeaders = 0;
        for (; responseHeaders[numHeaders * 2]; ++numHeaders)
        {
            std::string strName = std::string(responseHeaders[numHeaders * 2]);
            std::string strVal = std::string(responseHeaders[(numHeaders * 2) + 1]);
            strVal.erase(std::remove(strVal.begin(), strVal.end(), '\n'), strVal.end());
            strVal.erase(0, strVal.find_first_not_of(" "));
            strVal.erase(strVal.find_last_not_of(" ") + 1);
            if (strcmp(strName.c_str(), "content-length") == 0)
            {
                size_t pos = 0;
                uint64_t val = std::stoull(strVal.c_str(), &pos);
                if (pos == 0)
                {
                    file_size = 0;
                }
                else
                {
                    file_size = val;
                }
            }
        }
        av_log(NULL, AV_LOG_DEBUG, "fetch:get file size = %lld,name = %s.\n", file_size, filename);
        emscripten_fetch_free_unpacked_response_headers(responseHeaders);
    }
    else
    {
        av_log(NULL, AV_LOG_DEBUG, "fetch:get file size failed,name = %s ,status =%d.\n", filename, fetch->status);
        emscripten_fetch_close(fetch);
        return -1;
    }
    emscripten_fetch_close(fetch);
    return file_size;
}

int wasm_fech(char *filename, int64_t pos, int64_t to, uint8_t *buf)
{
    int read_size = -1;
    char strRange[50];
    sprintf(strRange, "bytes=%lld-%lld", pos, to);
    const char *headers[] = {"Range", strRange, nullptr};

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.requestHeaders = headers;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;

    emscripten_fetch_t *fetch = emscripten_fetch(&attr, filename);
    if (fetch->status == 200 || fetch->status == 206)
    {
        if (fetch->numBytes > 0)
        {
            memcpy(buf, fetch->data, fetch->numBytes);
            read_size = fetch->numBytes;
        }
        else
        {
            av_log(NULL, AV_LOG_DEBUG, "fetch:read file failed,name = %s ,status =%d.\n", filename, fetch->status);
        }
    }
    else
    {
        av_log(NULL, AV_LOG_ERROR, "fetch:read file failed,name = %s ,status =%d.\n", filename, fetch->status);
    }
    emscripten_fetch_close(fetch);
    return read_size;
}