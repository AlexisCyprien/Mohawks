#ifndef HTTP_PARSER__H
#define HTTP_PARSER__H

typedef struct {
    char *method;
    char *uri;
    char *version;
} request_line;

typedef struct {
    char *name;
    char *field;
    header *next;
} header;

typedef struct {
    request_line *request_line;
    header *headers;
    char *body;
} http_request;

#define CRLF "\r\n"
#define SPACE " "
#define ENDREQ "\r\n\r\n"

#define ERR_SPACE "  "

int check_request(char *rawdata);

int parse_request_line(char *rawdata, http_request *request);

int parse_http_request(char *rawdata, http_request *request);

#endif  // HTTP_PARSER__H
