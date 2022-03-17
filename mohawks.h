#ifndef MOHAWKS__H
#define MOHAWKS__H

#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

#include <stdbool.h>

#define SERVER_NAME "Mohawks/0.9"

#define DEFAULT_CONTENT_DIR "./content"
#define DEFAULT_INDEX "index.html"
#define HTTP_DATE_FORMAT "%a, %d %b %Y %T %Z"

#define HTTP_RESP_SIZE 4096

#define HTTP_VERSION "HTTP/1.0"

#define OK_STATUS "200 OK"

#define REDIRECT_STATUS "301 Moved Permanently"
#define NOT_MODIFIED_STATUS "304 Not Modified"

#define BAD_REQUEST_STATUS "400 Bad Request"
#define FORBIDEN_STATUS "403 Forbidden"
#define NOT_FOUND_STATUS "404 Not Found"
#define TIMEOUT_STATUS "408 Request Timeout"

#define INTERNAL_ERROR_STATUS "500 Internal Server Error"
#define NOT_IMPLEMENTED_STATUS "501 Not Implemented"

#define EXPIRE_TIME 3600

typedef struct status_line {
    char *version;
    char *status_code;
} status_line;

typedef struct http_response {
    status_line *status_line;
    header *headers;
    char *body;
    unsigned long body_size;
} http_response;

// Lance le serveur
int run_server(void);

// Traite la connexion au client
void *treat_connection(void *arg);

// Traite la requète HTTP
int treat_http_request(SocketTCP *sservice, http_request *request);

// Traite la requète de méthode GET
int treat_GET_HEAD_request(SocketTCP *sservice, http_request *request);

// Remplie la stucture http_response avec la version, le status, le corps et la taille du corps
int create_http_response(http_response *response, const char *version, 
        const char *status, const char *body, unsigned long bodysize);

// Ajoute le header constitué de name et de field à response
int add_response_header(const char *name, const char *field, http_response *response);

// Envoie la structure http_response
int send_http_response(SocketTCP *osocket, http_response *response);

// Libère la mémoire allouée par une structure http_response
void free_http_response(http_response *response);

// Envoie une réponse sans corps ni en-têtes
int send_simple_response(SocketTCP *osocket, const char *status);

// Envoie une réponse 200 OK
int send_200_response(SocketTCP *osocket, http_response *response);

// Envoie une réponse 301 Moved Permanently
int send_301_response(SocketTCP *osocket, char *new_path);

// Envoie une réponse 304 Not Modified
int send_304_response(SocketTCP *osocket);

// Envoie une réponse 400 Bad Request
int send_400_response(SocketTCP *osocket);

// Envoie une réponse 404 Not Found
int send_404_response(SocketTCP *osocket);

// Envoie une réponse 403 Forbidden
int send_403_response(SocketTCP *osocket);

// Envoie une réponse 408 Request Timeout
int send_408_response(SocketTCP *osocket);

// Envoie une réponse 500 Internal Server Error
int send_500_response(SocketTCP *osocket);

// Envoie une réponse 501 Not Implemented
int send_501_response(SocketTCP *osocket);

bool is_modified_since(http_request *request, time_t mod_date);

#endif  // MOHAWKS__H