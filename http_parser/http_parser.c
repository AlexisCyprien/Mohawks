#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "http_parser.h"

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_request(http_request *request) {
    if (request == NULL) {
        return -1;
    }
    request->request_line = NULL;
    request->headers = NULL;
    request->body = NULL;

    return 0;
}

int check_blank_line_request(char *rawdata) {
    if (rawdata == NULL) {
        return ERR_NULL;
    }

    // Ligne vide, fin de la requête
    if (strstr(rawdata, ENDREQ) == NULL) {
        return ERR_BLANK_LINE;
    }
    // Double espace
    if (strstr(rawdata, ERR_SPACE) != NULL) {
        return -1;
    }
    return 0;
}

int parse_request_line(char *rawdata, http_request *request) {
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

    char *name = NULL;
    char *field = NULL;

    if (add_headers(name, field, request) != 0) {
        return -1;
    }

    return 0;
}

int parse_http_request(char *rawdata, http_request *request) {
    if (rawdata == NULL || request == NULL) {
        return ERR_NULL;
    }

    int r = check_blank_line_request(rawdata);
    if (r != 0) {
        return r;
    }

    // Découpage des sauts de ligne
    char *saveptr;
    char *token = strtok_r(rawdata, CRLF, &saveptr);
    if (token == NULL) {
        return ERR_NULL;
    }

    // Traitement Request-Line
    char *reqline = malloc(strlen(token) + 1);
    if (reqline == NULL) {
        return -1;
    }
    strncpy(reqline, token, strlen(token) + 1);

    if (parse_request_line(reqline, request) == -1) {
        free(reqline);
        return -1;
    }
    free(reqline);
    reqline = NULL;

    // Traitement Headers
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

    return 0;
}

int add_headers(char *name, char *field, http_request *request) {
    if (name == NULL || field == NULL || request == NULL) {
        return ERR_NULL;
    }
    header **pp = &(request->headers);

    while (pp != NULL) {
        if (strcmp(name, (*pp)->name) == 0) {
            return ERR_REQUEST;
        }
        pp = &((*pp)->next);
    }
    pp = malloc(sizeof *pp);
    if (pp == NULL) {
        return ERR_MALLOC;
    }
    (*pp)->name = malloc(strlen(name) + 1);
    if ((*pp)->name == NULL) {
        return ERR_MALLOC;
    }
    snprintf((*pp)->name, strlen(name) + 1, "%s", name);

    (*pp)->field = malloc(strlen(field) + 1);
    if ((*pp)->field == NULL) {
        return ERR_MALLOC;
    }
    snprintf((*pp)->field, strlen(field) + 1, "%s", field);

    return 0;
}

void free_headers(header *headers) {
    while (headers != NULL) {
        header *t = headers;
        headers = headers->next;
        free(t->field);
        free(t->name);
        free(t);
    }
}

void free_http_request(http_request *request) {
    free(request->request_line->method);
    free(request->request_line->uri);
    free(request->request_line->version);
    free(request->request_line);
    free_headers(request->headers);
    free(request->body);
    free(request);
}
