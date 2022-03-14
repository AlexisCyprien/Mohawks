#ifndef HTTP_SERVER__H
#define HTTP_SERVER__H

#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

#define SERVER_NAME "Mohawks/0.9\r\n"

#define DEFAULT_CONTENT_DIR "./content"
#define DEFAULT_INDEX "index.html"
#define DEFAULT_DATE_FORMAT "%a, %d %b %Y %T %Z"

#define HTTP_RESP_SIZE 4096

#define FORBIDEN_RESP "HTTP/1.0 403 Forbidden\r\n\r\n"
#define NOT_FOUND_RESP "HTTP/1.0 404 Not Found\r\n\r\n"
#define OK_RESP "HTTP/1.0 200 OK\r\n"
#define TIMEOUT_RESP "HTTP/1.1 408 Request Timeout\r\n\r\n"

int run_server(void);

void *treat_connection(void *arg);

int treat_http_request(SocketTCP *sservice, http_request *request);

int treat_GET_request(SocketTCP *sservice, http_request *request);

#endif  // HTTP_SERVER__H