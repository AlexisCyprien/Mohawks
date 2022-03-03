#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "http_parser.h"

#include <regex.h>
#include <stdio.h>
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

    // Verification de la chaine via la regex et découpage par groupe
    regex_t regex;
    regmatch_t pmatch[REGEX_RL_MATCH];

    if (regcomp(&regex, REGEX_REQ_LINE, REG_EXTENDED) != 0) {
        fprintf(stderr, "regcomp");
        return -1;
    }
    int errcode = regexec(&regex, rawdata, REGEX_RL_MATCH, pmatch, 0);
    if (errcode != 0) {
        char errbuf[20];
        regerror(errcode, &regex, errbuf, 20);
        fprintf(stderr, "regexec : %s \n", errbuf);
        return -1;
    }

    // Ajout des champs de la chaine verifiee dans la structure request
    size_t method_len = (size_t)(pmatch[1].rm_eo - pmatch[1].rm_so);
    request->request_line->method = malloc(method_len);
    if (request->request_line->method == NULL) {
        return -1;
    }
    strncpy(request->request_line->method, rawdata + pmatch[1].rm_so,
            method_len);
    *(request->request_line->method + method_len) = '\0';

    size_t uri_len = (size_t)(pmatch[2].rm_eo - pmatch[2].rm_so);
    request->request_line->uri = malloc(uri_len);
    if (request->request_line->uri == NULL) {
        return -1;
    }
    strncpy(request->request_line->uri, rawdata + pmatch[2].rm_so, uri_len);
    *(request->request_line->uri + uri_len) = '\0';

    size_t version_len = (size_t)(pmatch[3].rm_eo - pmatch[3].rm_so);
    request->request_line->version = malloc(uri_len);
    if (request->request_line->version == NULL) {
        return -1;
    }
    strncpy(request->request_line->version, rawdata + pmatch[3].rm_so,
            version_len);
    *(request->request_line->version + version_len) = '\0';

    return 0;
}

int parse_header(char *rawdata, http_request *request) {
    if (rawdata == NULL || request == NULL) {
        return -1;
    }
    regex_t regex;
    regmatch_t pmatch[REGEX_HD_MATCH];

    if (regcomp(&regex, REGEX_HEADERS, REG_EXTENDED) != 0) {
        fprintf(stderr, "regcomp");
        return -1;
    }
    int errcode = regexec(&regex, rawdata, REGEX_HD_MATCH, pmatch, 0);
    if (errcode != 0) {
        char errbuf[20];
        regerror(errcode, &regex, errbuf, 20);
        fprintf(stderr, "regexec : %s \n", errbuf);
        return -1;
    }

    char *name;
    char *field;

    if (request_add_headers(name, field, request) != 0) {
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
    reqline = NULL;

    // Headers

    while (token != NULL) {
        token = strtok_r(NULL, CRLF, &saveptr);

        char *header = malloc(strlen(token) + 1);
        strncpy(header, token, strlen(token) + 1);
        if (parse_header(header, request) == -1) {
            free(header);
            return -1;
        }
        free(header);
        header = NULL;
    }

    // Body

    return 0;
}
