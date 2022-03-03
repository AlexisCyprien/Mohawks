#include "http_server.h"

#include <stdio.h>
#include <stdlib.h>

#include "adresse_internet/adresse_internet.h"
#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

int main(void) {
    if (run_server() != 0) {
        fprintf(stderr, "Erreur lancement serveur HTTP");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int run_server(void) {
    adresse_internet *addr_in = adresse_internet_loopback(80);
    if (addr_in == NULL) {
        return -1;
    }

    SocketTCP *socket = malloc(sizeof *socket);
    if (socket == NULL) {
        adresse_internet_free(addr_in);
        return -1;
    }

    return 0;
}