/******************************************************************************
 *                              MOHAWKS :
 *     Bibliothèque C permettant la gestion d'un serveur HTTP/1.0 et la création
 *     d'une structure pour les réponse HTTP aux clients.
 ******************************************************************************/

#ifndef MOHAWKS__H
#define MOHAWKS__H

#include <stdbool.h>

#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

/*******************************************************************************
 *                                 MACROS
 ******************************************************************************/

#define SERVER_PORT 80
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
#define CONNECTION_TIMEOUT 30000

/*******************************************************************************
 *                                 STRUCTURES
 ******************************************************************************/

// status_line : Structure pouvant contenir les champs d'une ligne de
//                status HTTP
typedef struct status_line {
    char *version;
    char *status_code;
} status_line;

// http_response : Structure contenant les champs d'une reponse HTTP
typedef struct http_response {
    status_line *status_line;
    header *headers;
    char *body;
    unsigned long body_size;
} http_response;

/*******************************************************************************
 *                                 FONCTIONS
 ******************************************************************************/

// run_server : Lance le serveur
extern int run_server(void);

// treat_connection : Traite la connexion au client
extern void *treat_connection(void *arg);

// treat_http_request : Traite la requète HTTP request avec les réponses renvoyé
//                    sur la socket de service service.
extern int treat_http_request(SocketTCP *sservice, http_request *request);

// treat_GET_HEAD_request : Traite la requète de méthode GET ou HEAD avec
//                        les réponses renvoyé sur la socket de service service.
extern int treat_GET_HEAD_request(SocketTCP *sservice, http_request *request);

// create_http_response : Remplie la stucture http_response avec la version,
//                        le status, le corps et la taille du corps
extern int create_http_response(http_response *response, const char *version,
                                const char *status, const char *body,
                                unsigned long bodysize);

// add_response_header : Ajoute le header constitué de name et de field
//                       à la structure response
extern int add_response_header(const char *name, const char *field,
                               http_response *response);

// send_http_response : Envoie la structure http_response via la socket osocket
extern int send_http_response(SocketTCP *osocket, http_response *response);

// free_http_response : Libère les ressources de la structure http_response
extern void free_http_response(http_response *response);

// send_simple_response : Envoie une réponse HTTP sans corps ni en-têtes
extern int send_simple_response(SocketTCP *osocket, const char *status);

// send_200_response : Envoie une réponse 200 OK sur la socket TCP osocket
extern int send_200_response(SocketTCP *osocket, http_response *response);

// send_301_response : Envoie une réponse 301 Moved Permanently
//                     sur la socket TCP osocket
extern int send_301_response(SocketTCP *osocket, char *new_path);

// send_304_response : Envoie une réponse 304 Not Modified sur la socket TCP
extern int send_304_response(SocketTCP *osocket);

// send_400_response : Envoie une réponse 400 Bad Request sur la socket TCP
extern int send_400_response(SocketTCP *osocket);

// send_400_response : Envoie une réponse 404 Not Found sur la socket TCP
extern int send_404_response(SocketTCP *osocket);

// send_403_response : Envoie une réponse 403 Forbidden sur la socket TCP
extern int send_403_response(SocketTCP *osocket);

// send_408_response : Envoie une réponse 408 Request Timeout sur la socket TCP
extern int send_408_response(SocketTCP *osocket);

// send_500_response : Envoie une réponse 500 Internal Server Error sur la
//                     socket TCP
extern int send_500_response(SocketTCP *osocket);

// send_501_response : Envoie une réponse 501 Not Implemented sur la socket TCP
extern int send_501_response(SocketTCP *osocket);

// is_modified_since : Renvoie si l'en-tête If-Modified-Since est plus
//                     récente que mod_date
extern bool is_modified_since(http_request *request, time_t mod_date);

#endif  // MOHAWKS__H