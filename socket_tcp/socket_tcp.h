/******************************************************************************
 *                              SOCKET_TCP :
 *     Bibliothèque C permettant la manipulation de socket
 ******************************************************************************/

#ifndef SOCKET_TCP__H
#define SOCKET_TCP__H

#include <stdbool.h>
#include <time.h>

#include "../adresse_internet/adresse_internet.h"
#include "../adresse_internet/adresse_internet_type.h"

/*******************************************************************************
 *                                 MACROS
 ******************************************************************************/

#define SIZE_QUEUE 16

/*******************************************************************************
 *                                 STRUCTURES
 ******************************************************************************/

typedef struct SocketTCP {
    int sockfd;
    adresse_internet *local;
    adresse_internet *distant;
    bool connected;
    bool listening;
    bool bound;
} SocketTCP;

/*******************************************************************************
 *                                 FONCTIONS
 ******************************************************************************/

extern int initSocketTCP(SocketTCP *psocket);

extern int connectSocketTCP(SocketTCP *osocket, const char *adresse,
                            uint16_t port);

extern int creerSocketEcouteTCP(SocketTCP *isocket, const char *adresse,
                                uint16_t port);

extern int acceptSocketTCP(const SocketTCP *secoute, SocketTCP *sservice);

extern ssize_t writeSocketTCP(const SocketTCP *osocket, const char *buffer,
                              size_t length);

extern ssize_t readSocketTCP(const SocketTCP *nsocket, void *buffer,
                             size_t length);

extern int closeSocketTCP(SocketTCP *socket);

#endif
