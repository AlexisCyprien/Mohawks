#include "unit_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../http_parser/http_parser.h"

#define TEST_FUN_PRL(arg)                                              \
    do {                                                               \
        printf("---------- test_parse_request_line ----------\n");     \
        int r = test_parse_request_line(arg);                          \
        if (r != 0) {                                                  \
            printf("%s \n", err_to_string(r));                         \
        } else {                                                       \
            printf("Test réussis ! \n");                               \
        }                                                              \
        printf("-------------------------------------------------\n"); \
    } while (0)

int main(void) {
    TEST_FUN_PRL(GOOD_REQUEST_LINE);
    TEST_FUN_PRL(BAD_REQUEST_LINE);
    return EXIT_SUCCESS;
}

int test_parse_request_line(const char *rawdata) {
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