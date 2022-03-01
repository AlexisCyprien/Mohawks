#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "http_parser.h"

#include <stdlib.h>
#include <string.h>

int check_request(char *rawdata) {
    if (rawdata == NULL) {
        return -1;
    }

    // Ligne vide, fin de la requête
    if (strstr(rawdata, ENDREQ) == NULL) {
        return -1;
    }
    // Double espace
    if (strstr(rawdata, ERR_SPACE) != NULL) {
        return -1;
    }
    return 0;
}

int parse_resquest_line(char *rawdata, http_request *request) {
    if (rawdata == NULL || request == NULL) {
        return -1;
    }

    return 0;
}

int parse_http_request(char *rawdata, http_request *request) {
    if (rawdata == NULL || request == NULL) {
        return -1;
    }
    if (check_request(rawdata) == -1) {
        return -1;
    }
    // Découpage des sauts de ligne
    char *saveptr;
    char *token = strtok_r(rawdata, CRLF, &saveptr);
    if (token == NULL) {
        return -1;
    }

    // Traitement request line
    char *reqline = malloc(strlen(token) + 1);
    if (reqline == NULL) {
        return -1;
    }
    strncpy(reqline, token, strlen(token) + 1);

    if (parse_resquest_line(reqline, request) == -1) {
        free(reqline);
        return -1;
    }
    free(reqline);

    // Headers

    // Body

    return 0;
}
