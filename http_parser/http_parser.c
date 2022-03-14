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
    struct request_line *reqline = malloc(256);
    if (reqline == NULL) {
        return -1;
    }

    request->request_line = reqline;
    request->headers = NULL;
    request->body = NULL;

    return 0;
}

int check_blank_line_request(char *rawdata) {
    if (rawdata == NULL) {
        return ERR_NULL;
    }

    // Ligne vide, fin de la requête
    // if (strstr(rawdata, ENDREQ) == NULL) {
    //     return ERR_BLANK_LINE;
    // }
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
        regfree(&regex);
        fprintf(stderr, "regcomp");
        return -1;
    }
    int errcode = regexec(&regex, rawdata, REGEX_RL_MATCH, pmatch, 0);
    if (errcode != 0) {
        char errbuf[20];
        regfree(&regex);
        regerror(errcode, &regex, errbuf, 20);
        fprintf(stderr, "regexec : %s , %s \n", errbuf, __func__);
        return -1;
    }

    // Ajout des champs de la chaine verifiee dans la structure request
    size_t method_len = (size_t)(pmatch[1].rm_eo - pmatch[1].rm_so);
    request->request_line->method = malloc(method_len + 1);
    if (request->request_line->method == NULL) {
        return -1;
    }
    strncpy(request->request_line->method, rawdata + pmatch[1].rm_so,
            method_len);
    *(request->request_line->method + method_len) = '\0';

    size_t uri_len = (size_t)(pmatch[2].rm_eo - pmatch[2].rm_so);
    request->request_line->uri = malloc(uri_len + 1);
    if (request->request_line->uri == NULL) {
        return -1;
    }
    strncpy(request->request_line->uri, rawdata + pmatch[2].rm_so, uri_len);
    *(request->request_line->uri + uri_len) = '\0';

    size_t version_len = (size_t)(pmatch[3].rm_eo - pmatch[3].rm_so);
    request->request_line->version = malloc(version_len + 1);
    if (request->request_line->version == NULL) {
        return -1;
    }
    strncpy(request->request_line->version, rawdata + pmatch[3].rm_so,
            version_len);
    *(request->request_line->version + version_len) = '\0';

    regfree(&regex);
    return 0;
}

int parse_header(char *rawdata, http_request *request) {
    int r = 0;
    if (rawdata == NULL || request == NULL) {
        r = ERR_NULL;
        goto parse_end;
    }
    regex_t regex;
    regmatch_t pmatch[REGEX_HD_MATCH];

    if (regcomp(&regex, REGEX_HEADERS, REG_EXTENDED) != 0) {
        fprintf(stderr, "%s : regcomp\n", __func__);
        r = ERR_REGEX;
        goto parse_end;
    }

    int errcode = regexec(&regex, rawdata, REGEX_HD_MATCH, pmatch, 0);
    if (errcode != 0) {
        char errbuf[20];
        regerror(errcode, &regex, errbuf, 20);
        fprintf(stderr, "regexec : %s , %s \n", errbuf, __func__);
        r = ERR_REGEX;
        goto err_reg;
    }

    size_t name_len = (size_t)(pmatch[1].rm_eo - pmatch[1].rm_so);
    char *name = malloc(name_len + 1);
    if (name == NULL) {
        r = ERR_MALLOC;
        goto err_reg;
    }
    strncpy(name, rawdata + pmatch[1].rm_so, name_len);
    *(name + name_len) = '\0';

    size_t field_len = (size_t)(pmatch[2].rm_eo - pmatch[1].rm_so);
    char *field = malloc(field_len + 1);
    if (field == NULL) {
        r = ERR_MALLOC;
        goto err_malloc;
    }
    strncpy(field, rawdata + pmatch[2].rm_so, field_len);
    *(field + field_len) = '\0';

    r = add_headers(name, field, request);

    free(field);
    field = NULL;
err_malloc:
    free(name);
    name = NULL;
err_reg:
    regfree(&regex);
parse_end:
    return r;
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
    // char *reqline = malloc(strlen(token) + 1);
    char *reqline = malloc(256);
    if (reqline == NULL) {
        return -1;
    }
    // printf("SIZEOF REQLINE %ld \n", sizeof(reqline));
    // printf("STRLEN TOKEN %ld \n", strlen(token) + 1);
    // strncpy(reqline, token, sizeof(*reqline));
    // strncpy(reqline, token, strlen(token) + 1);
    memcpy(reqline, token, 256);
    if (parse_request_line(reqline, request) == -1) {
        free(reqline);
        return -1;
    }

    free(reqline);

    // Traitement Headers
    token = strtok_r(NULL, CRLF, &saveptr);
    while (token != NULL) {
        if (parse_header(token, request) == -1) {
        }
        token = strtok_r(NULL, CRLF, &saveptr);
    }
    return 0;
}

int add_headers(char *name, char *field, http_request *request) {
    if (name == NULL || field == NULL || request == NULL) {
        return ERR_NULL;
    }
    header **pp = &(request->headers);

    struct header *header = malloc(sizeof(struct header));
    if (header == NULL) {
        return -1;
    }
    header->name = malloc(strlen(name) + 1);
    if (header->name == NULL) {
        return ERR_MALLOC;
    }
    snprintf(header->name, strlen(name) + 1, "%s", name);
    header->field = malloc(strlen(field) + 1);
    if (header->field == NULL) {
        return ERR_MALLOC;
    }
    snprintf(header->field, strlen(field) + 1, "%s", field);
    header->next = NULL;

    if (*pp == NULL) {
        *pp = header;
    } else {
        while ((*pp)->next != NULL) {
            // printf("strcmp\n");
            // printf("header: %s\n", (*pp)->name);
            // if ((*pp)->name != NULL) {
            //     if (strcmp(name, (*pp)->name) == 0) {
            //         return ERR_REQUEST;
            //     }
            // }
            pp = &((*pp)->next);
        }
        (*pp)->next = header;
    }

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
    if (request == NULL) {
        return;
    }
    if (request->request_line != NULL) {
        free(request->request_line->method);
        free(request->request_line->uri);
        free(request->request_line->version);
        free(request->request_line);
    }
    if (request->headers != NULL) {
        free_headers(request->headers);
    }
    if (request->body != NULL) {
        free(request->body);
    }
    free(request);
}
