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

enum ERR { ERR_MALLOC = 1, ERR_REGEX, ERR_ };

#define CRLF "\r\n"
#define SPACE " "
#define ENDREQ "\r\n\r\n"

#define ERR_SPACE "  "

#define REGEX_REQ_LINE "^(GET|PUT|HEAD) (\\S+) (HTTP\\/[0-9].[0-9])"
#define REGEX_RL_MATCH 4

int check_request(char *rawdata);

// parse_request_line : Lit une chaine de caractères, vérifie si celle-ci est
//      une ligne de requête HTTP (Request-Line cf. RFC 1945) et,
//      dans le cas échéant ajoute les champs dans request.
//      Sinon, renvoie -1 en cas de requêtes non conforme, ou une erreur.
int parse_request_line(char *rawdata, http_request *request);

int parse_http_request(char *rawdata, http_request *request);

#endif  // HTTP_PARSER__H
