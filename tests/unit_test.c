#include "unit_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../http_parser/http_parser.h"

#define TEST_FUN(fun) printf("---------- " #fun " ----------\n")
#define TEST_END printf("-------------------------------------------------\n")

int main(void) {
    TEST_FUN(test_parse_request_line());
    int r = test_parse_request_line(GOOD_REQUEST_LINE);
    if (r != 0) {
        printf("%s \n", err_to_string(r));
    } else {
        printf("Test réussis ! \n");
    }
    TEST_END;

    TEST_FUN(test_parse_request_line());
    r = test_parse_request_line(BAD_REQUEST_LINE);
    if (r != 0) {
        printf("%s \n", err_to_string(r));
        printf("Test réussis ! \n");
    } else {
        printf("Test raté ! \n");
    }
    TEST_END;

    TEST_FUN(test_parse_header());
    r = test_parse_header(GOOD_HEADER);
    if (r != 0) {
        printf("%s \n", err_to_string(r));
    } else {
        printf("Test réussis ! \n");
    }
    TEST_END;

    TEST_FUN(test_parse_header());
    r = test_parse_header(BAD_HEADER);
    if (r != 0) {
        printf("%s \n", err_to_string(r));
        printf("Test réussis ! \n");
    } else {
        printf("Test raté ! \n");
    }
    TEST_END;
    TEST_FUN(test_parse_http_request());
    r = test_parse_http_request(
        GOOD_HTTP_REQUEST(GOOD_REQUEST_LINE, GOOD_HEADER));
    if (r != 0) {
        printf("%s \n", err_to_string(r));
    } else {
        printf("Test réussis ! \n");
    }
    TEST_END;
    TEST_FUN(test_parse_http_request());
    r = test_parse_http_request(BAD_HTTP_REQUEST(BAD_REQUEST_LINE, BAD_HEADER));
    if (r != 0) {
        printf("%s \n", err_to_string(r));
        printf("Test réussis ! \n");
    } else {
        printf("Test raté ! \n");
    }
    TEST_END;
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
    init_request(phttp);

    char test_request[60];
    strncpy(test_request, rawdata, sizeof(test_request) - 1);

    r = parse_request_line(test_request, phttp);
    if (r == 0) {
        printf("Method : %s \n", phttp->request_line->method);
        printf("URI : %s \n", phttp->request_line->uri);
        printf("Version : %s \n", phttp->request_line->version);
    }

    free_http_request(phttp);
tested:
    return r;
}

int test_parse_header(const char *rawdata) {
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
    init_request(phttp);

    char test_request[200];
    strncpy(test_request, rawdata, sizeof(test_request) - 1);

    r = parse_header(test_request, phttp);
    if (r == 0) {
        printf("Header : %s \n", phttp->headers->name);
        printf("Field : %s \n", phttp->headers->field);
    }

    free_http_request(phttp);
tested:
    return r;
}

int test_parse_http_request(const char *rawdata) {
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
    init_request(phttp);

    char test_request[200];
    strncpy(test_request, rawdata, sizeof(test_request) - 1);

    r = parse_http_request(test_request, phttp);
    if (r == 0) {
        printf("Method : %s \n", phttp->request_line->method);
        printf("URI : %s \n", phttp->request_line->uri);
        printf("Version : %s \n", phttp->request_line->version);
        printf("Header : %s \n", phttp->headers->name);
        printf("Field : %s \n", phttp->headers->field);
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