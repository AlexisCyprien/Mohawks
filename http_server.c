#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "http_server.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "adresse_internet/adresse_internet.h"
#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"

void handler(int num);

SocketTCP *secoute;

int main(void) {
    signal(SIGHUP, SIG_IGN);
    /*TODO: implémenter la gestion des signaux */
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTERM);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    struct sigaction sa;
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (run_server() != 0) {
        fprintf(stderr, "Erreur  serveur HTTP\n");  // Traiter erreurs
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int run_server(void) {
    secoute = malloc(sizeof *secoute);
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

    int ret = poll(fds, 1, 30000);  // Timeout 10s

    if (ret == -1) {
        perror("poll");
    } else if (ret == 0) {
        // Timeout à gerer
        if (writeSocketTCP(sservice, TIMEOUT_RESP, sizeof(TIMEOUT_RESP)) ==
            -1) {
            // Err
        }
        if (closeSocketTCP(sservice) == -1) {
            // Err
        }
        pthread_exit(NULL);
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

            int r = parse_http_request(buffer, request);
            if (r != 0) {
                // Err
                pthread_exit(NULL);
            }

            r = treat_http_request(sservice, request);
            if (r != 0) {
                // Err
                if (closeSocketTCP(sservice) == -1) {
                    // Err
                }

                pthread_exit(NULL);
            }

            free(buffer);
            // free etc
        }
    }
    pthread_exit(NULL);
}

int treat_http_request(SocketTCP *sservice, http_request *request) {
    if (sservice == NULL || request == NULL) {
        return ERR_NULL;
    }

    // Vérification de l'uri demandé pour se protéger des attaques par
    // traversée de répertoire.
    char *ret = strstr(request->request_line->uri, "../");
    if (ret != NULL) {
        if (writeSocketTCP(sservice, FORBIDEN_RESP, sizeof(FORBIDEN_RESP)) ==
            -1) {
            closeSocketTCP(sservice);
            return -1;
        }
        if (closeSocketTCP(sservice) == -1) {
            return -1;
        }
    } else {
        if (strcmp(request->request_line->method, "GET") == 0) {
            if (treat_GET_request(sservice, request) == -1) {
                return -1;
            }
        }
    }
    return 0;
}

void handler(int num) {
    switch (num) {
        case SIGINT:
            if (secoute != NULL) {
                closeSocketTCP(secoute);
            }
            exit(EXIT_SUCCESS);
        case SIGTERM:
            if (secoute != NULL) {
                closeSocketTCP(secoute);
            }
            exit(EXIT_SUCCESS);
    }
}
int treat_GET_request(SocketTCP *sservice, http_request *request) {
    if (sservice == NULL || request == NULL) {
        return ERR_NULL;
    }

    // On construit le chemin du fichier demandé
    char path[PATH_MAX] = DEFAULT_CONTENT_DIR;
    strncat(path, request->request_line->uri, sizeof(path) - 1);
    if (strcmp(request->request_line->uri, "/") == 0) {
        strncat(path, DEFAULT_INDEX, sizeof(path) - 1);
    }
    printf("path: %s\n", path);

    // On construit le header Date
    char date_name[] = "Date: ";
    char date_field[200];
    time_t t;
    struct tm readable_time;
    if (time(&t) == (time_t)-1) {
        perror("time");
    }
    localtime_r(&t, &readable_time);

    strftime(date_field, sizeof(date_field), "%a, %d %b %Y %T %Z",
             &readable_time);
    char date_header[strlen(date_name) + strlen(date_field) + strlen(CRLF) + 1];
    strncpy(date_header, date_name, sizeof(date_header) - 1);
    strcat(date_header, date_field);
    strcat(date_header, CRLF);

    // On construit le header Server
    char serv_name[SERVER_HEADER_NAME_SIZE] = "Server: ";
    char server_header[SERVER_HEADER_NAME_SIZE + strlen(SERVER_NAME) + 1];
    strncpy(server_header, serv_name, sizeof(server_header) - 1);
    strcat(server_header, SERVER_NAME);

    // On construit la réponse
    char resp[HTTP_RESP_SIZE];
    strncpy(resp, OK_RESP, sizeof(resp) - 1);
    strncat(resp, date_header, sizeof(resp) - 1);
    strncat(resp, server_header, sizeof(resp) - 1);
    // strncat(resp, "Content-Type: image/png\r\n", sizeof(resp) -1);

    // On récupère le fichier à envoyer
    int index_fd;
    errno = 0;
    // Si le fichier demandé n'existe pas, on envoie un code 404
    if ((index_fd = open(path, O_RDONLY)) == -1) {
        if (errno == ENOENT) {
            if (writeSocketTCP(sservice, NOT_FOUND_RESP,
                               sizeof(NOT_FOUND_RESP)) == -1) {
                closeSocketTCP(sservice);
                return -1;
            }
            if (closeSocketTCP(sservice) == -1) {
                return -1;
            }
            return 0;
        } else {
            return -1;
        }
    }

    // On récupère la taille du fichier
    struct stat filestat;
    if (fstat(index_fd, &filestat) == -1) {
        perror("fstat");
        closeSocketTCP(sservice);
        return -1;
    }

    char filesize[100];
    snprintf(filesize, sizeof(filesize) - 1, "%ld\r\n\r\n", filestat.st_size);

    strncat(resp, "Content-Length: ", sizeof(resp) - 1);
    strncat(resp, filesize, sizeof(resp) - 1);

    char file[filestat.st_size + 1];
    ssize_t n;
    if ((n = read(index_fd, file, sizeof(file))) == -1) {
        closeSocketTCP(sservice);
    }
    if (close(index_fd) == -1) {
        closeSocketTCP(sservice);
        return -1;
    }
    printf("bytes: %ld\n", (long)n);

    // On construit le corps de la réponse
    char body[sizeof(resp) + sizeof(file) + (sizeof(CRLF) * 2) + 1];
    strncpy(body, resp, sizeof(body) - 1);
    memcpy(&body[strlen(resp)], file, sizeof(file) - 1);
    strncat(body, CRLF, sizeof(body) - 1);
    strncat(body, CRLF, sizeof(body) - 1);
    body[sizeof(body) - 1] = 0;

    // On envoie la réponse
    printf("%s", body);
    if (writeSocketTCP(sservice, body, sizeof(body)) == -1) {
        closeSocketTCP(sservice);
        return -1;
    }
    if (closeSocketTCP(sservice) == -1) {
        return -1;
    }
    printf("envoyé\n");
    return 0;
}
