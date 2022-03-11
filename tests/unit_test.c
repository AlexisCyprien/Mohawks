#include "unit_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../http_parser/http_parser.h"

#define TEST_FUN(fun) printf("---------- " #fun " ----------\n")
#define TEST_END printf("-------------------------------------------------\n")

int main(void) {
    TEST_FUN(test_parse_request_line());
    char test_request[60];
    strncpy(test_request, GOOD_REQUEST_LINE, strlen(GOOD_REQUEST_LINE) + 1);
    int r = test_parse_request_line(test_request);
    if (r != 0) {
        printf("%s \n", err_to_string(r));
    } else {
        printf("Test réussis ! \n");
    }
    TEST_END;

    TEST_FUN(test_parse_request_line());
    memset(test_request, 0, sizeof test_request);
    strncpy(test_request, BAD_REQUEST_LINE, strlen(BAD_REQUEST_LINE) + 1);
    r = test_parse_request_line(test_request);
    if (r != 0) {
        printf("Test réussis ! \n");
        printf("%s \n", err_to_string(r));
    } else {
        printf("Test raté ! \n");
    }
    TEST_END;
    return EXIT_SUCCESS;
}

int test_parse_request_line(char *rawdata) {
    int r = 0;
    if (rawdata == NULL) {
        r = ERR_NULL;
        goto tested;
    }

    http_request *phttp = malloc(sizeof *phttp);
    if (phttp == NULL) {
        r = ERR_MALLOC;
        goto tested;
    }
    memset(phttp, 0, sizeof *phttp);

    phttp->request_line = malloc(sizeof phttp->request_line);
    if (phttp->request_line == NULL) {
        r = ERR_MALLOC;
        goto tested;
    }
    r = parse_request_line(rawdata, phttp);
    if (r == 0) {
        printf("Method : %s \n", phttp->request_line->method);
        printf("URI : %s \n", phttp->request_line->uri);
        printf("Version : %s \n", phttp->request_line->version);
    }

    free_http_request(phttp);
tested:
    return r;
}

const char *err_to_string(int err) {
    switch (err) {
        case ERR_MALLOC:
            return "Erreur d'allocation mémoire avec malloc.";
            break;
        case ERR_NULL:
            return "Erreur d'initialisation : valeur à NULL.";
            break;
        case ERR_REGEX:
            return "Erreur de fonctions regex.";
            break;
        case ERR_REQUEST:
            return "Erreur requête invalide.";
            break;
        default:
            return "Erreur ? Erreur !";
            break;
    }
}