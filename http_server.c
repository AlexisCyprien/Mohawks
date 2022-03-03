#include "http_server.h"

#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "adresse_internet/adresse_internet.h"
#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

int main(void) {
    if (run_server() != 0) {
        fprintf(stderr, "Erreur  serveur HTTP");  // Traiter erreurs
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int run_server(void) {
    SocketTCP *secoute = malloc(sizeof *secoute);
    if (secoute == NULL) {
        return -1;
    }
    if (creerSocketEcouteTCP(secoute, "localhost", 80) != 0) {  // localhost ??
        closeSocketTCP(secoute);
        return -1;
    }

    while (1) {
        SocketTCP *sservice = malloc(sizeof *sservice);
        if (sservice == NULL) {
            closeSocketTCP(secoute);
            return -1;
        }

        if (acceptSocketTCP(secoute, sservice) == -1) {
            closeSocketTCP(secoute);
            return -1;
        }

        pthread_t th;
        if (pthread_create(&th, NULL, treat_connection, sservice) != 0) {
            fprintf(stderr, " Erreur \n");
            return -1;
        }
        if (pthread_detach(th) == -1) {
            perror("pthread_detach");
            return -1;
        }
    }

    return 0;
}

void *treat_connection(void *arg) {
    if (arg == NULL) {
        return;
    }
    SocketTCP *sservice = (SocketTCP *)arg;
    struct pollfd fds[1];

    fds[0].fd = sservice->sockfd;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 10000);  // Timeout 10s

    if (ret == -1) {
        perror("poll");
    } else if (ret == 0) {
        // Timeout à gerer

    } else {
        if (fds[0].revents & POLLERR) {
            // Erreur sur la socket
        }

        if (fds[0].revents & POLLIN) {
            size_t buflen = 4096;
            char buffer = malloc(buflen);
            if (buffer = NULL) {
                // Erreur à gerer
                pthread_exit(EXIT_FAILURE);
            }

            if (readSocketTCP(sservice, buffer, buflen) == -1) {
                // Err
                pthread_exit(EXIT_FAILURE);
            }
            // Lecture requete

            http_request *request = malloc(sizeof *request);
            if (request == NULL) {
                // Err
                pthread_exit(EXIT_FAILURE);
            }
            if (init_request(request) != 0) {
                // Err
                pthread_exit(EXIT_FAILURE);
            }

            int r = parse_http_request(buffer, request);
            if (r != 0) {
                // Err
                pthread_exit(EXIT_FAILURE);
            }

            r = treat_http_request(sservice, request);
            if (r != 0) {
                // Err
                pthread_exit(EXIT_FAILURE);
            }

            // free etc
        }
    }

    // Lecture requete

    // Traitement requete

    // Bye bye
}

int treat_http_request(SocketTCP *sservice, http_request *request) {
    if (sservice == NULL || request == NULL) {
        return ERR_NULL;
    }

    // Lire requete et envoyer ou non contenu selon celle ci

    return 0;
}