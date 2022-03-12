#ifndef HTTP_PARSER__H
#define HTTP_PARSER__H

typedef struct request_line request_line;

typedef struct request_line {
    char *method;
    char *uri;
    char *version;
} request_line;

typedef struct header header;

typedef struct header {
    char *name;
    char *field;
    header *next;
} header;

typedef struct http_request http_request;

typedef struct http_request {
    request_line *request_line;
    header *headers;
    char *body;
} http_request;

enum ERR {
    ERR_MALLOC = 1,
    ERR_NULL,
    ERR_REGEX,
    ERR_REQUEST,
    ERR_BLANK_LINE,
};

#define CRLF "\r\n"
#define SPACE " "
#define ENDREQ "\r\n\r\n"

#define ERR_SPACE "  "

#define REGEX_REQ_LINE "^(GET|HEAD|POST) (\\S+) (HTTP\\/[0-9].[0-9])"
#define REGEX_RL_MATCH 4

// #define REGEX_HEADERS                                                          
//     "^[(Date|Pragma|Authorization|From|If-Modified-Since|Referer|User-Agent)]: " 
//     "(\\S+)"
#define REGEX_HEADERS "(\\S+): (\\S+)"

#define REGEX_HD_MATCH 3

/*
     FONCTIONS STRUCTURES
*/

// init_request : Initialisation de la structure http_request
//      Renvoie -1 si request est à null
int init_request(http_request *request);

// add_headers : Ajoute un header dans la structure request si name et field
//      ne sont pas présent dans la structure.
int add_headers(char *name, char *field, http_request *request);

// free_http_request : Libère les ressources de la structure http_request
//      de request.
void free_http_request(http_request *request);

/*
     FONCTIONS PARSING
*/

// check_blank_line_request : Teste si il y a la ligne vide en fin de requête.
//      Renvoie 0 en cas de succès, sinon une erreur de type ERR.
int check_blank_line_request(char *rawdata);

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

// parse_http_request : Lit une chaine de caracteres, et verifie si celle-ci est
//      une requete HTTP conforme et la traite en la stockant dans request.
//      Renvoie 0 en cas de succès sinon une erreur de type ERR.
int parse_http_request(char *rawdata, http_request *request);

#endif  // HTTP_PARSER__H
