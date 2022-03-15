#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "mime_type.h"

#include <string.h>
#include <stdbool.h>

bool endswith(const char *s1, const char *s2) {
    size_t s1len = strlen(s1);
    size_t s2len = strlen(s2);
    return (s1len >= s2len) && (strcmp(s1 + s1len - s2len, s2) == 0);;
}

const char *get_mime_type(const char *filename) {
    if (endswith(filename, ".html") 
        || endswith(filename, ".htm")
        || endswith(filename, ".htmls")) {
        return "text/html";
    } else if (endswith(filename, ".css")) {
        return "text/css";
    } else if (endswith(filename, ".js")) {
        return "text/javascript";
    } else if (endswith(filename, ".jpg") 
            || endswith(filename, ".jpeg")) {
        return "image/jpeg";
    } else if (endswith(filename, ".png")) {
        return "image/png";
    } else if (endswith(filename, ".svg")) {
        return "image/svg+xml";
    } else if (endswith(filename, ".gif")) {
        return "image/gif";
    } else if (endswith(filename, ".bmp")) {
        return "image/bmp";
    } else if (endswith(filename, ".ico")) {
        return "image/x-icon";
    } else if (endswith(filename, ".png")) {
        return "image/png";
    } else if (endswith(filename, ".mp2")
            || endswith(filename, ".mp3")) {
        return "audio/mpeg";
    } else if (endswith(filename, ".wav")) {
        return "audio/wav";
    } else if (endswith(filename, ".mp4")) {
        return "video/mp4";
    } else if (endswith(filename, ".bin")
            || endswith(filename, ".dms")
            || endswith(filename, ".lrf")
            || endswith(filename, ".mar")
            || endswith(filename, ".so")
            || endswith(filename, ".pkg")
            || endswith(filename, ".dump")
            || endswith(filename, ".bkp")) {
        return "application/octet-stream";
    } else return "text/plain";
}