#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "adresse_internet.h"

#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "adresse_internet_type.h"

adresse_internet *adresse_internet_new(const char *nom, uint16_t port) {
    adresse_internet *p = malloc(sizeof p);
    if (p == NULL) {
        perror("malloc");
        return NULL;
    }

    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    size_t port_length = (size_t)((ceil(log10(port)) + 1) * sizeof(char));
    char strport[port_length + 1];
    snprintf(strport, port_length + 1, "%hu", port);
    int r;
    if ((r = getaddrinfo(nom, strport, &hints, &result)) != 0) {
        free(p);
        freeaddrinfo(result);
        fprintf(stderr, "getaddrinfo : %s", gai_strerror(r));
        return NULL;
    }
    memcpy(&(p->sock_addr), result->ai_addr, result->ai_addrlen - 1);
    snprintf(p->nom, strlen(nom) + 1, "%s", nom);
    snprintf(p->service, strlen(strport) + 1, "%s", strport);

    freeaddrinfo(result);

    return p;
}

adresse_internet *adresse_internet_any(uint16_t port) {
    char strany[sizeof INADDR_ANY];
    snprintf(strany, sizeof(INADDR_ANY), "%hu", INADDR_ANY);

    return adresse_internet_new(strany, port);
}

adresse_internet *adresse_internet_loopback(uint16_t port) {
    char strloopback[sizeof INADDR_LOOPBACK];
    snprintf(strloopback, sizeof(INADDR_LOOPBACK), "%hu", INADDR_LOOPBACK);

    return adresse_internet_new(strloopback, port);
}

void adresse_internet_free(adresse_internet *adresse) {
    if (adresse == NULL) {
        return;
    }
    free(adresse);
}

int adresse_internet_get_info(adresse_internet *adresse, char *nom_dns,
                              int taille_dns, char *nom_port, int taille_port) {
    if (nom_dns == NULL && nom_port == NULL) {
        return -1;
    }
    struct sockaddr *addr = (struct sockaddr *)&(adresse->sock_addr);
    int r;
    if ((r = getnameinfo(addr, (socklen_t)sizeof(adresse->sock_addr), nom_dns,
                         (socklen_t)taille_dns, nom_port,
                         (socklen_t)taille_port, 0) != 0)) {
        fprintf(stderr, "getaddrinfo : %s", gai_strerror(r));
        return -1;
    }
    return 0;
}

int adresse_internet_get_ip(const adresse_internet *adresse, char *ip,
                            int taille_ip) {
    if (adresse->sock_addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *socktest =
            (struct sockaddr_in6 *)&(adresse->sock_addr);
        inet_ntop(AF_INET6, &(socktest->sin6_addr), ip, (socklen_t)taille_ip);
    } else {
        struct sockaddr_in *socktest =
            (struct sockaddr_in *)&(adresse->sock_addr);
        inet_ntop(AF_INET, &(socktest->sin_addr), ip, (socklen_t)taille_ip);
    }

    return 0;
}

uint16_t adresse_internet_get_port(const adresse_internet *adresse) {
    if (adresse == NULL) {
        return 0;
    }

    char *endptr;
    errno = 0;
    u_int16_t port = (u_int16_t)strtoul(adresse->service, &endptr, 10);
    if (errno != 0) {
        perror("strtol");
        return 0;
    }

    if (endptr == adresse->service) {
        fprintf(stderr, "Port/Adresse invalide\n");
        return 0;
    }

    return port;
}

int adresse_internet_get_domain(const adresse_internet *adresse) {
    if (adresse == NULL) {
        return -1;
    }
    return adresse->sock_addr.ss_family;
}
int sockaddr_to_adresse_internet(const struct sockaddr *addr,
                                 adresse_internet *adresse) {
    if (adresse == NULL) {
        return -1;
    }
    if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *tmpaddr6 = (struct sockaddr_in6 *)addr;
        memcpy(&(adresse->sock_addr), &tmpaddr6, sizeof tmpaddr6);
    } else {
        struct sockaddr_in *tmpaddr = (struct sockaddr_in *)addr;
        memcpy(&(adresse->sock_addr), &tmpaddr, sizeof tmpaddr);
    }
    return 0;
}

int adresse_internet_to_sockaddr(const adresse_internet *adresse,
                                 struct sockaddr *addr) {
    if (adresse == NULL || addr == NULL) {
        return -1;
    }
    if (sizeof(adresse->sock_addr) > sizeof(*addr)) {
        return -1;
    }
    memcpy(&addr, &(adresse->sock_addr), sizeof(adresse->sock_addr));

    return 0;
}

int adresse_internet_compare(const adresse_internet *adresse11,
                             const adresse_internet *adresse22) {
    if (adresse11 == NULL || adresse22 == NULL) {
        return -1;
    }
    return strcmp(adresse11->nom, adresse22->nom) &&
                   strcmp(adresse11->service, adresse22->service)
               ? 1
               : 0;
}

int adresse_internet_copy(adresse_internet *adrdst,
                          const adresse_internet *adrsrc) {
    if (adrsrc == NULL) {
        return -1;
    }
    memcpy(&(adrdst->sock_addr), &(adrsrc->sock_addr),
           sizeof(adrsrc->sock_addr));
    memcpy(&(adrdst->nom), &(adrsrc->nom), sizeof(adrsrc->nom));
    memcpy(&(adrdst->service), &(adrsrc->service), sizeof(adrsrc->service));
    return 0;
}
