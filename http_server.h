#ifndef HTTP_SERVER__H
#define HTTP_SERVER__H

#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

int run_server(void);

void *treat_connection(void *arg);

int treat_http_request(SocketTCP *sservice, http_request *request);

#endif  // HTTP_SERVER__H