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
#include <dirent.h>

#include "adresse_internet/adresse_internet.h"
#include "http_parser/http_parser.h"
#include "socket_tcp/socket_tcp.h"
#include "mime_type/mime_type.h"
#include "directory_index/dir_index.h"

void handler(int num);

SocketTCP *secoute;
static __thread SocketTCP *sservice;

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
    initSocketTCP(secoute);
    if (creerSocketEcouteTCP(secoute, "localhost", 80) != 0) {
        fprintf(stderr, "Erreur creerSocketEcouteTCP. \n");
        closeSocketTCP(secoute);
        return -1;
    }

    while (1) {
        sservice = malloc(sizeof *sservice);  // <- PAS FREE !!
        initSocketTCP(sservice);
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
        return NULL;
    }
    SocketTCP *sservice = (SocketTCP *)arg;
    struct pollfd fds[1];

    fds[0].fd = sservice->sockfd;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 30000);  // Timeout 30s

    if (ret == -1) {
        perror("poll");
    } else if (ret == 0) {
        // Gestion du timeout
        http_response *response = malloc(sizeof(http_response));
        if (create_http_response(response, HTTP_VERSION, TIMEOUT_STATUS, NULL, 0) == -1) {
            free_http_response(response);
            closeSocketTCP(sservice);
        }
        if (send_http_response(sservice, response) == -1) {
            free_http_response(response);
            closeSocketTCP(sservice);
        }
        free_http_response(response);

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
                free(buffer);
                pthread_exit(NULL);
            }
            // Lecture requete

            http_request *request = malloc(sizeof *request);
            if (request == NULL) {
                // Err
                if (closeSocketTCP(sservice) == -1) {
                    // Err
                }
                free(buffer);
                pthread_exit(NULL);
            }
            if (init_request(request) != 0) {
                // Err
                if (closeSocketTCP(sservice) == -1) {
                    // Err
                }
                goto end_connection;
            }

            int r = parse_http_request(buffer, request);
            if (r != 0) {
                http_response *response = malloc(sizeof(http_response));
                if (create_http_response(response, HTTP_VERSION, BAD_REQUEST_STATUS, NULL, 0) == -1) {
                    free_http_response(response);
                    closeSocketTCP(sservice);
                }
                if (send_http_response(sservice, response) == -1) {
                    free_http_response(response);
                    closeSocketTCP(sservice);
                }
                free_http_request(request);
                free_http_response(response);

                pthread_exit(NULL);
            }

            r = treat_http_request(sservice, request);
            if (r != 0) {
                // Err
                if (closeSocketTCP(sservice) == -1) {
                    // Err
                }
            }
        end_connection:
            free_http_request(request);
            request = NULL;
            free(buffer);
            buffer = NULL;
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
        http_response *response = malloc(sizeof(http_response));
        if (response == NULL) {
            return -1;
        }
        if (create_http_response(response, HTTP_VERSION, FORBIDEN_STATUS, NULL, 0) == -1) {
            closeSocketTCP(sservice);
            return -1;
        }
        if (send_http_response(sservice, response) == -1) {
            free_http_response(response);
            closeSocketTCP(sservice);
            return -1;
        }
        free_http_response(response);
        return 0;

    } else {
        if (strcmp(request->request_line->method, "GET") == 0) {
            if (treat_GET_request(sservice, request) == -1) {
                return -1;
            }
        } else {
            http_response *response = malloc(sizeof(http_response));
            if (response == NULL) {
                return -1;
            }
            if (create_http_response(response, HTTP_VERSION, NOT_IMPLEMENTED_STATUS, NULL, 0) == -1) {
                closeSocketTCP(sservice);
                return -1;
            }
            if (send_http_response(sservice, response) == -1) {
                free_http_response(response);
                closeSocketTCP(sservice);
                return -1;
            }
            free_http_response(response);
            return 0;
        }
    }
    return 0;
}

int treat_GET_request(SocketTCP *sservice, http_request *request) {
    if (sservice == NULL || request == NULL) {
        return ERR_NULL;
    }

    // On construit le chemin du fichier demandé
    char path[PATH_MAX] = DEFAULT_CONTENT_DIR;
    strncat(path, request->request_line->uri, sizeof(path) - 1);
    if (endswith(request->request_line->uri, "/")) {
        strncat(path, DEFAULT_INDEX, sizeof(path) - 1);
    }

    // On récupère le fichier à envoyer
    int index_fd;
    errno = 0;
    // Si le fichier demandé n'existe pas, on envoie un code 404
    if ((index_fd = open(path, O_RDONLY)) == -1) {
        if (errno == ENOENT) {
            // Le fichier demandé n'existe pas
            if (endswith(request->request_line->uri, "/")) {
                directory_index(request, path, sservice);
                return 0;
            }

            http_response *response = malloc(sizeof(http_response));
            if (response == NULL) {
                return -1;
            }
            if (create_http_response(response, HTTP_VERSION, NOT_FOUND_STATUS, NULL, 0) == -1) {
                closeSocketTCP(sservice);
                return -1;
            }
            if (send_http_response(sservice, response) == -1) {
                free_http_response(response);
                closeSocketTCP(sservice);
                return -1;
            }
            free_http_response(response);
            return 0;
        } else if (errno == EACCES) {
            // L'accès au fichiuer demandé n'est pas autorisé 
            http_response *response = malloc(sizeof(http_response));
            if (response == NULL) {
                return -1;
            }
            if (create_http_response(response, HTTP_VERSION, FORBIDEN_STATUS, NULL, 0) == -1) {
                closeSocketTCP(sservice);
                return -1;
            }
            if (send_http_response(sservice, response) == -1) {
                free_http_response(response);
                closeSocketTCP(sservice);
                return -1;
            }
            free_http_response(response);
            return 0;
        } else {
            closeSocketTCP(sservice);
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
    snprintf(filesize, sizeof(filesize) - 1, "%ld", filestat.st_size);

    char file[filestat.st_size + 1];
    ssize_t n;
    if ((n = read(index_fd, file, sizeof(file))) == -1) {
        closeSocketTCP(sservice);
    }
    if (close(index_fd) == -1) {
        closeSocketTCP(sservice);
        return -1;
    }
    file[sizeof(file) - 1] = 0;

    // On construit la réponse
    http_response *response = malloc(sizeof(http_response));
    if (response == NULL) {
        return -1;
    }
    if (create_http_response(response, HTTP_VERSION, OK_STATUS,
            file, (unsigned long) filestat.st_size) == -1) {
        closeSocketTCP(sservice);
        return -1;
    }
    // On construit le header Date
    char date_field[200];
    time_t t;
    struct tm readable_time;
    if (time(&t) == (time_t)-1) {
        perror("time");
    }
    localtime_r(&t, &readable_time);

    strftime(date_field, sizeof(date_field), "%a, %d %b %Y %T %Z",
             &readable_time);
    add_response_header("Date", date_field, response);

    // On construit le header Server
    add_response_header("Server", SERVER_NAME, response);

    // On construit le header Content-Type
    const char *mime = get_mime_type(path);
    add_response_header("Content-Type", mime, response);

    // On construit le header Content-Length
    add_response_header("Content-Length", filesize, response);

    if (send_http_response(sservice, response) == -1) {
        free_http_response(response);
        closeSocketTCP(sservice);
        return -1;
    }
    free_http_response(response);

    return 0;
}


void handler(int num) {
    switch (num) {
        case SIGINT:
            if (secoute != NULL) {
                closeSocketTCP(secoute);
            }
            if (sservice != NULL) {
                closeSocketTCP(sservice);
            }
            exit(EXIT_SUCCESS);
        case SIGTERM:
            if (secoute != NULL) {
                closeSocketTCP(secoute);
            }
            if (sservice != NULL) {
                closeSocketTCP(sservice);
            }
            exit(EXIT_SUCCESS);
    }
}


int create_http_response(http_response *response, const char *version, 
        const char *status, const char *body, unsigned long body_size) {
    if (response == NULL || version == NULL || status == NULL) {
        return -1;
    }
    status_line *status_line = malloc(sizeof(struct status_line));
    if (status_line == NULL) {
        return -1;
    }
    status_line->version = malloc(sizeof(version));
    if (status_line->version == NULL) {
        return -1;
    }
    status_line->status_code = malloc(sizeof(status));
    if (status_line->status_code == NULL) {
        return -1;
    }

    memcpy(status_line->version, version, strlen(version));
    memcpy(status_line->status_code, status, strlen(status));
    response->status_line = status_line;
    response->headers = NULL;
    if (body != NULL) {
            response->body = malloc(body_size + 1);
            if (response->body == NULL) {
                return -1;
            }
            memcpy(response->body, body, body_size);
            response->body_size = body_size;
    } else { 
        response->body = NULL; 
        response->body_size = 0;    
    }

    return 0;
}


int send_http_response(SocketTCP *osocket, http_response *response) {
    if (osocket == NULL || response == NULL) {
        return -1;
    }
    
    char resp[HTTP_RESP_SIZE];
    char *version = response->status_line->version;
    char *status = response->status_line->status_code;

    // On ajoute la version http
    strncpy(resp, version, sizeof(resp) - 1);

    // On ajoute le status
    strncat(resp, status, sizeof(resp) -1);
    strncat(resp, CRLF, sizeof(resp) -1);
    
    // On ajoute les headers
    if (response->headers != NULL) {
        header **pp = &(response->headers);
        while (*pp != NULL) {
            char *name = (*pp)->name;
            char *field = (*pp)->field;

            // On ajoute le nom du header
            strncat(resp, name, sizeof(resp) -1);

            // On ajoute le champ du header
            strncat(resp, field, sizeof(resp) -1);

            strncat(resp, CRLF, sizeof(resp) -1);
            
            pp = &((*pp)->next);
        }
    }

    strncat(resp, CRLF, sizeof(resp) -1);

    if (writeSocketTCP(osocket, resp, strlen(resp)) == -1){
        return -1;
    }
    if (writeSocketTCP(osocket, response->body, response->body_size) == -1){
        return -1;
    }
    if (response->body != NULL) {
        for (int i = 0; i < 2; ++i) {
            if (writeSocketTCP(osocket, CRLF, strlen(CRLF)) == -1){
                return -1;
            }
        }
    }   
    
    if (closeSocketTCP(osocket) == -1) {
        return -1;
    }

    return 0;
}


void free_http_response(http_response *response) {
    if (response == NULL) {
        return;
    }
    if (response->status_line != NULL) {
        free(response->status_line->version);
        free(response->status_line->status_code);
        free(response->status_line);
    }
    if (response->headers != NULL) {
        free_headers(response->headers);
    }
    if (response->body != NULL) {
        free(response->body);
    }
    free(response);
}


int add_response_header(const char *name, const char *field, http_response *response) {
    if (name == NULL || field == NULL || response == NULL) {
        return ERR_NULL;
    }
    header **pp = &(response->headers);

    struct header *header = malloc(sizeof(struct header));
    if (header == NULL) {
        return -1;
    }
    header->name = malloc(strlen(name) + strlen(": ") + 1);
    if (header->name == NULL) {
        return ERR_MALLOC;
    }
    snprintf(header->name, strlen(name) + 1, "%s", name);
    strncat(header->name, ": ", sizeof(header->name) -1);
    header->field = malloc(strlen(field) + 1);
    if (header->field == NULL) {
        return ERR_MALLOC;
    }
    snprintf(header->field, strlen(field) + 1, "%s", field);
    header->next = NULL;

    if (*pp == NULL) {
        *pp = header;
    } else {
        while ((*pp)->next != NULL) {
            pp = &((*pp)->next);
        }
        (*pp)->next = header;
    }

    return 0;
}

