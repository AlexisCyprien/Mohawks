#include "http_server.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "adresse_internet/adresse_internet.h"
#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

int main(void) {
    if (run_server() != 0) {
        fprintf(stderr, "Erreur  serveur HTTP\n");  // Traiter erreurs
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
        fprintf(stderr, "Erreur creerSocketEcouteTCP. \n");
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
        printf("Connexion acceptée\n");
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
        return NULL;
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
            char *buffer = malloc(buflen);
            if (buffer == NULL) {
                // Erreur à gerer
                pthread_exit(NULL);
            }

            if (readSocketTCP(sservice, buffer, buflen) == -1) {
                // Err
                pthread_exit(NULL);
            }
            printf("requête: %s\n", buffer);
            // Lecture requete

            http_request *request = malloc(sizeof *request);
            if (request == NULL) {
                // Err
                pthread_exit(NULL);
            }
            if (init_request(request) != 0) {
                // Err
                pthread_exit(NULL);
            }
            printf("requête initialisée!\n");

            int r = parse_http_request(buffer, request);
            if (r != 0) {
                // Err
                pthread_exit(NULL);
            }

            r = treat_http_request(sservice, request);
            if (r != 0) {
                // Err
                pthread_exit(NULL);
            }

            free(buffer);
            // free etc
        }
    }
    return NULL;
    // Bye bye
}

int treat_http_request(SocketTCP *sservice, http_request *request) {
    if (sservice == NULL || request == NULL) {
        return ERR_NULL;
    }

    if (strcmp(request->request_line->method, "GET") == 0) {
        char *ret = strstr(request->request_line->uri, "../");
        if (ret != NULL) {
            if (writeSocketTCP(sservice, FORBIDEN_RESP, sizeof(FORBIDEN_RESP)) == -1) {
                //Err
            }
            if (closeSocketTCP(sservice) == -1) {
                //Err
            }
        } else {
            char path[PATH_MAX] = ".";
            strncat(path, request->request_line->uri, sizeof(path) - 1);
            strncat(path, INDEX, sizeof(path) - 1);

            time_t t;
            if (time(&t) == (time_t)-1) {
                perror("time");
            }
            struct tm readable_time;
            localtime_r(&t, &readable_time);
            char time[200] = "Date: ";
            strftime(&time[strlen(time)], sizeof(time), "%a, %d %b %Y %T %Z",
                     &readable_time);
            strncat(time, CRLF, sizeof(time) - 1);

            char server[200] = "Server: ";
            strncat(server, SERVER_NAME, sizeof(server) - 1);

            char resp[4096];
            strncpy(resp, OK_RESP, sizeof(resp));
            strncat(resp, time, sizeof(resp) - 1);
            strncat(resp, server, sizeof(resp) - 1);

            int index_fd;
            errno = 0;
            if ((index_fd = open(path, O_RDONLY)) == -1) {
                if (errno == ENOENT) {
                    if (send(sservice->sockfd, NOT_FOUND_RESP,
                             sizeof(NOT_FOUND_RESP),
                             0) < (ssize_t)sizeof(NOT_FOUND_RESP)) {
                        // Err
                    }
                } else {
                    // Err
                }
            }
            char body[2048];
            if (read(index_fd, &body, sizeof(body)) == -1) {
                // Err
            }
            strncat(resp, CRLF, sizeof(resp) - 1);
            strncat(resp, body, sizeof(resp) - 1);
            strncat(resp, CRLF, sizeof(resp) - 1);
            strncat(resp, CRLF, sizeof(resp) - 1);

            if (writeSocketTCP(sservice, resp, sizeof(resp)) == -1) {
                //Err
            }
            if (closeSocketTCP(sservice) == -1) {
                //Err
            }
            

            // if (send(sservice->sockfd, &resp, sizeof(resp), 0) == 1) {
            //     // Err
            // }

            pthread_exit(NULL);
        }
    }

    return 0;
}