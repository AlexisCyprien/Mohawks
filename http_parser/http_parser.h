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

enum ERR {
    ERR_MALLOC = 1,
    ERR_NULL,
    ERR_REGEX,
    ERR_REQUEST,
    ERR_END_REQUEST,
};

#define CRLF "\r\n"
#define SPACE " "
#define ENDREQ "\r\n\r\n"

#define ERR_SPACE "  "

#define REGEX_REQ_LINE "^(GET|HEAD|POST) (\\S+) (HTTP\\/[0-9].[0-9])"
#define REGEX_RL_MATCH 4

#define REGEX_HEADERS                                                          \
    "^(Date|Pragma|Authorization|From|If-Modified-Since|Referer|User-Agent): " \
    "(\\S+)"

#define REGEX_HD_MATCH 2

int check_request(char *rawdata);

// parse_request_line : Lit une chaine de caractères, vérifie si celle-ci est
//      une ligne de requête HTTP (Request-Line cf. RFC 1945) et,
//      dans le cas échéant ajoute les champs dans request.
//      Sinon, renvoie -1 en cas de requêtes non conforme, ou une erreur.
int parse_request_line(char *rawdata, http_request *request);

// parse_header : Lit une chaine de caractères,
//      vérifie si celle-ci est un header de requête HTTP
//      (General-Header, Request-Header , Entity-Header cf. RFC 1945) et,
//      dans le cas échéant ajoute les champs dans request.
//      Sinon, renvoie -1 en cas de requêtes non conforme, ou une erreur.
int parse_header(char *rawdata, http_request *request);

int parse_http_request(char *rawdata, http_request *request);

#endif  // HTTP_PARSER__H
