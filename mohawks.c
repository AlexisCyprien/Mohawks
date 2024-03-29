#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "mohawks.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "adresse_internet/adresse_internet.h"
#include "directory_index/dir_index.h"
#include "http_parser/http_parser.h"
#include "mime_type/mime_type.h"
#include "socket_tcp/socket_tcp.h"

// display_help : Affiche de l'aide sur la sortie standard
static void display_help(void);
// signal_setup : Mise en place de la gestion des signaux pour le serveur
static int signal_setup(void);
// handler : Gestionnaire des signaux du serveur
static void handler(int num);

int port = SERVER_PORT;
char log_filename[50] = LOGFILE_NAME;
FILE *logfile;

static __thread SocketTCP *secoute;
static __thread SocketTCP *sservice;

int main(int argc, char *argv[]) {
    /* Gestion des options */
    int opt;
    char *endptr;
    while ((opt = getopt(argc, argv, "hp:o:")) != -1) {
        switch (opt) {
            case 'h':
                /* Help */
                display_help();
                return EXIT_SUCCESS;
                break;
            case 'p':
                /* Port */
                errno = 0;
                port = (int)strtol(optarg, &endptr, 10);

                if ((errno == ERANGE && (port == INT_MAX || port == INT_MIN)) ||
                    (errno != 0 && port == 0)) {
                    perror("strtol");
                    fprintf(stderr, "Usage: %s [-p port] [-o filename] \n",
                            argv[0]);
                    exit(EXIT_FAILURE);
                }

                if (endptr == optarg || port <= 0 || port >= 65535) {
                    fprintf(stderr, "Port invalide. \n");
                    fprintf(stderr, "Usage: %s [-p port] [-o filename] \n",
                            argv[0]);
                    exit(EXIT_FAILURE);
                }

                break;
            case 'o':
                /* Log Filename */
                strncpy(log_filename, optarg, 49);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-p port] [-o filename] \n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind > argc) {
        fprintf(stderr, "Expected argument after options.\n");
        fprintf(stderr, "Usage: %s [-p port] [-o filename] \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Gestion des signaux */
    signal_setup();
    /* Lancement du serveur */
    if (run_server() != 0) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static void display_help(void) {
    printf("Mohawks/0.9 \n\n");
    printf("Usage: ./mohawks [-h] [-p port] [-o filename]\n");
    printf("Configuration par défaut : \n");
    printf("\t Port : %d \n", SERVER_PORT);
    printf("\t Log filename : %s \n", LOGFILE_NAME);
    printf("Si votre port est entre 0 et 1023, oubliez pas sudo !\n");
}

static int signal_setup(void) {
    sigset_t mask;
    if (sigfillset(&mask) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigfillset");
        return EXIT_FAILURE;
    }
    if (sigdelset(&mask, SIGINT) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigdelset");
        return EXIT_FAILURE;
    }
    if (sigdelset(&mask, SIGTERM) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigdelset");
        return EXIT_FAILURE;
    }
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    struct sigaction sa;
    sa.sa_handler = handler;
    if (sigfillset(&sa.sa_mask) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigfillset");
        return EXIT_FAILURE;
    }
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigfillset");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("sigfillset");
        return EXIT_FAILURE;
    }
    return 0;
}

static void handler(int num) {
    switch (num) {
        case SIGINT:
            if (secoute != NULL) {
                closeSocketTCP(secoute);
            }
            if (sservice != NULL) {
                closeSocketTCP(sservice);
            }
            if (fclose(logfile)) {
                perror("fclose");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        case SIGTERM:
            if (secoute != NULL) {
                closeSocketTCP(secoute);
            }
            if (sservice != NULL) {
                closeSocketTCP(sservice);
            }
            if (fclose(logfile)) {
                perror("fclose");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
    }
}

int run_server(void) {
    logfile = fopen(log_filename, "a");
    if (logfile == NULL) {
        fprintf(stderr, "Erreur : Lancement du serveur HTTP\n");
        perror("fopen");
        return EXIT_FAILURE;
    }

    secoute = malloc(sizeof *secoute);
    if (secoute == NULL) {
        return -1;
    }
    initSocketTCP(secoute);
    if (creerSocketEcouteTCP(secoute, "0.0.0.0", (uint16_t)port) != 0) {
        fprintf(stderr, "Erreur creerSocketEcouteTCP. \n");
        closeSocketTCP(secoute);
        return -1;
    }

    while (1) {
        sservice = malloc(sizeof *sservice);
        initSocketTCP(sservice);
        if (sservice == NULL) {
            closeSocketTCP(secoute);
            return -1;
        }

        if (acceptSocketTCP(secoute, sservice) == -1) {
            closeSocketTCP(secoute);
            return -1;
        }
        logger(logfile, sservice, "Has connected");
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

void logger(FILE *f, SocketTCP *socket, const char *msg) {
    if (f == NULL) {
        fprintf(stderr, "Error : log file not found ! \n");
    }
    time_t t;
    if (time(&t) == (time_t)-1) {
        perror("logtofile");
    }
    struct tm readable_time;
    localtime_r(&t, &readable_time);

    char time[200];
    strftime(time, sizeof(time), "%a %b %d %T %Y", &readable_time);
    fprintf(f, "%s - CLIENT : %s - %s \n", time, socket->distant->nom, msg);
}

void *treat_connection(void *arg) {
    if (arg == NULL) {
        return NULL;
    }
    SocketTCP *sservice = (SocketTCP *)arg;
    struct pollfd fds[1];

    fds[0].fd = sservice->sockfd;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, CONNECTION_TIMEOUT);

    if (ret == -1) {
        send_500_response(sservice);
        closeSocketTCP(sservice);
        pthread_exit(NULL);
    } else if (ret == 0) {
        // Gestion du timeout
        logger(logfile, sservice, "Has deconnected (Timeout)");
        send_408_response(sservice);
        closeSocketTCP(sservice);
        pthread_exit(NULL);
    } else {
        if (fds[0].revents & POLLERR) {
            // Erreur sur la socket
            logger(logfile, sservice, "Has deconnected (Socket Error)");
            closeSocketTCP(sservice);
            pthread_exit(NULL);
        }

        if (fds[0].revents & POLLIN) {
            size_t buflen = 4096;
            char *buffer = malloc(buflen);
            if (buffer == NULL) {
                send_500_response(sservice);
                pthread_exit(NULL);
            }

            ssize_t offset = 0;
            do {
                ssize_t n = readSocketTCP(sservice, buffer + offset,
                                          buflen - ((size_t)offset));
                if (n == -1) {
                    free(buffer);
                    if (closeSocketTCP(sservice) == -1) {
                        // Err
                    }
                    pthread_exit(NULL);
                }
                offset += n;
            } while (strstr(buffer, "\r\n\r\n") == NULL &&
                     strstr(buffer, "\n\n") == NULL &&
                     offset < (ssize_t)buflen);

            logger(logfile, sservice, buffer);

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
                if (r == ERR_REGEX) {
                    send_400_response(sservice);
                } else {
                    send_500_response(sservice);
                }
                goto end_connection;
            }
            r = treat_http_request(sservice, request);
            if (r != 0) {
                send_500_response(sservice);
            }

        end_connection:
            logger(logfile, sservice, "Has deconnected.");
            free_http_request(request);
            request = NULL;
            free(buffer);
            buffer = NULL;
            closeSocketTCP(sservice);
            pthread_exit(NULL);
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
        return send_403_response(sservice);
    } else {
        char *ret = strstr(request->request_line->uri, ".");
        if (ret == NULL) {
            // Si l'uri demandée est un dossier et qu'il manque le / final,
            // Une redirection est envoyée avec l'uri corrigée.
            if (!endswith(request->request_line->uri, "/")) {
                char new_uri[strlen(request->request_line->uri) + 2];
                snprintf(new_uri, sizeof(new_uri), "%s/",
                         request->request_line->uri);

                return send_301_response(sservice, new_uri);
            }
        }

        if (strcmp(request->request_line->method, "GET") == 0 ||
            strcmp(request->request_line->method, "HEAD") == 0) {
            return treat_GET_HEAD_request(sservice, request);
        } else
            return send_501_response(sservice);
    }
    return 0;
}

int treat_GET_HEAD_request(SocketTCP *sservice, http_request *request) {
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
    if ((index_fd = open(path, O_RDONLY)) == -1) {
        switch (errno) {
            // Le fichier demandé n'existe pas
            case ENOENT:
                // Si l'uri demandé est un dossier, on lance l'indexation
                if (endswith(request->request_line->uri, "/")) {
                    if (directory_index(request, path, sservice) == -1) {
                        return send_500_response(sservice);
                    } else
                        return 0;
                }

                // Sinon on envoie un code 404 Not Found
                return send_404_response(sservice);
                break;

            // Pas autorisé à accéder à cette entrée
            case EACCES:
            case EFAULT:
                return send_403_response(sservice);
                break;
            default:
                return send_500_response(sservice);
                break;
        }
    }
    // On récupère la taille du fichier
    struct stat filestat;
    if (fstat(index_fd, &filestat) == -1) {
        return -1;
    }
    char filesize[100];
    snprintf(filesize, sizeof(filesize) - 1, "%ld", filestat.st_size);

    if (!is_modified_since(request, filestat.st_mtim.tv_sec)) {
        return send_304_response(sservice);
    }

    // On construit la réponse selon la méthode de la requête
    http_response *response = malloc(sizeof(http_response));
    if (response == NULL) {
        return -1;
    }

    unsigned long len = (unsigned long)filestat.st_size;
    if (strcmp(request->request_line->method, "HEAD") == 0) {
        if (create_http_response(response, HTTP_VERSION, OK_STATUS, NULL,
                                 len) == -1) {
            return -1;
        }

        // On construit le header Content-Type
        const char *mime = get_mime_type(path);
        add_response_header("Content-Type", mime, response);

        if (send_200_response(sservice, response) == -1) {
            free_http_response(response);
            return -1;
        }
        free_http_response(response);
        if (close(index_fd) == -1) {
            return -1;
        }

        return 0;

    } else {
        // Nous utilisons la fonction mmap pour projeter le contenu du fichier
        // dans La mémoire vive de l'ordinateur. Cela permet d'envoyer des
        // fichiers bien plus gros qu'en lisant son contenu dans un tableau de
        // caractères avec l'appel read, puisqu'ici, la seule limite de taille
        // pour le fichier à envoyer est la mémoire disponnible sur le serveur.
        char *file = mmap(0, len, PROT_READ, MAP_SHARED, index_fd, 0);
        if (file == MAP_FAILED) return -1;

        if (create_http_response(response, HTTP_VERSION, OK_STATUS, file,
                                 len) == -1) {
            return -1;
        }

        // On construit le header Content-Type
        const char *mime = get_mime_type(path);
        add_response_header("Content-Type", mime, response);

        if (send_200_response(sservice, response) == -1) {
            munmap(file, len);
            free_http_response(response);
            return -1;
        }
        munmap(file, len);
        free_http_response(response);
        if (close(index_fd) == -1) {
            return -1;
        }

        return 0;
    }
}

int create_http_response(http_response *response, const char *version,
                         const char *status, const char *body,
                         unsigned long body_size) {
    if (response == NULL || version == NULL || status == NULL) {
        return -1;
    }

    // On construit la ligne de status de la réponse
    status_line *status_line = malloc(sizeof *status_line);
    if (status_line == NULL) {
        return -1;
    }
    status_line->version = malloc(strlen(version) + 1);
    if (status_line->version == NULL) {
        return -1;
    }
    status_line->status_code = malloc(strlen(status) + 1);
    if (status_line->status_code == NULL) {
        return -1;
    }

    // On y met la version de HTTP et le code de status
    memcpy(status_line->version, version, strlen(version) + 1);
    memcpy(status_line->status_code, status, strlen(status) + 1);
    response->status_line = status_line;

    // On initialise les headers de la réponse
    response->headers = NULL;

    // Si le corps n'est pas vide, on l'ajoute à notre réponse
    if (body != NULL) {
        response->body = malloc(body_size + 1);
        if (response->body == NULL) {
            return -1;
        }
        memcpy(response->body, body, body_size);
    } else {
        response->body = NULL;
    }
    response->body_size = body_size;
    return 0;
}

int send_http_response(SocketTCP *osocket, http_response *response) {
    if (osocket == NULL || response == NULL) {
        return -1;
    }

    // On créer notre buffer de réponse, il contiendra la ligne de status
    // et les en-têtes
    char resp[HTTP_RESP_SIZE];
    char *version = response->status_line->version;
    char *status = response->status_line->status_code;

    // On ajoute la version http
    snprintf(resp, strlen(version) + 2, "%s ", version);

    // On ajoute le status
    strncat(resp, status, sizeof(resp) - 1);
    strncat(resp, CRLF, sizeof(resp) - 1);

    // On ajoute les headers
    if (response->headers != NULL) {
        header **pp = &(response->headers);
        while (*pp != NULL) {
            char *name = (*pp)->name;
            char *field = (*pp)->field;

            // On ajoute le nom du header
            strncat(resp, name, sizeof(resp) - 1);

            // On ajoute le champ du header
            strncat(resp, field, sizeof(resp) - 1);

            strncat(resp, CRLF, sizeof(resp) - 1);

            pp = &((*pp)->next);
        }
    }

    strncat(resp, CRLF, sizeof(resp) - 1);

    // On envoie d'abord l'en-tête de la réponse
    if (writeSocketTCP(osocket, resp, strlen(resp)) == -1) {
        return -1;
    }
    if (response->body != NULL) {
        // Puis le corps de la réponse
        if (writeSocketTCP(osocket, response->body, response->body_size) ==
            -1) {
            return -1;
        }

        for (int i = 0; i < 2; ++i) {
            if (writeSocketTCP(osocket, CRLF, strlen(CRLF)) == -1) {
                return -1;
            }
        }
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

int add_response_header(const char *name, const char *field,
                        http_response *response) {
    if (name == NULL || field == NULL || response == NULL) {
        return ERR_NULL;
    }
    header **pp = &(response->headers);

    struct header *header = malloc(sizeof(struct header));
    if (header == NULL) {
        return -1;
    }
    header->name = malloc(strlen(name) + 3);
    if (header->name == NULL) {
        return ERR_MALLOC;
    }
    // On copie le nom dans le header
    snprintf(header->name, strlen(name) + 3, "%s: ", name);

    header->field = malloc(strlen(field) + 1);
    if (header->field == NULL) {
        return ERR_MALLOC;
    }
    // On copie le champs dans le header
    snprintf(header->field, strlen(field) + 1, "%s", field);

    // On ajoute le header en tête de la liste chaînée de la réponse
    if (*pp == NULL) {
        header->next = NULL;
        *pp = header;
    } else {
        header->next = (*pp)->next;
        (*pp)->next = header;
    }

    return 0;
}

int send_simple_response(SocketTCP *osocket, const char *status) {
    http_response *response = malloc(sizeof(struct http_response));
    if (response == NULL) {
        return -1;
    }
    // On créé la réponse avec un simple code de status
    if (create_http_response(response, HTTP_VERSION, status, NULL, 0) == -1) {
        free_http_response(response);
        return -1;
    }
    if (send_http_response(osocket, response) == -1) {
        free_http_response(response);
        return -1;
    }
    free_http_response(response);
    return 0;
}

int send_200_response(SocketTCP *osocket, http_response *response) {
    // On ajoute le header Content-Length
    char size[sizeof(unsigned long) + 1];
    snprintf(size, sizeof(size), "%ld", response->body_size);
    add_response_header("Content-Length", size, response);

    // On ajoute le header Server
    add_response_header("Server", SERVER_NAME, response);

    // On ajoute le header Date
    time_t t;
    if (time(&t) == (time_t)-1) return -1;
    struct tm readable_time;
    gmtime_r(&t, &readable_time);
    char date[200];
    strftime(date, sizeof(date), HTTP_DATE_FORMAT, &readable_time);
    add_response_header("Date", date, response);

    // On ajoute le header Expires
    t += EXPIRE_TIME;
    gmtime_r(&t, &readable_time);
    char expire_date[200];
    strftime(expire_date, sizeof(expire_date), HTTP_DATE_FORMAT,
             &readable_time);
    add_response_header("Expires", expire_date, response);

    // On ajoute le header Allow
    add_response_header("Allow", "GET, HEAD", response);

    return send_http_response(osocket, response);
}

int send_301_response(SocketTCP *osocket, char *new_path) {
    http_response *response = malloc(sizeof(struct http_response));
    if (response == NULL) {
        return -1;
    }
    if (create_http_response(response, HTTP_VERSION, REDIRECT_STATUS, NULL,
                             0) == -1) {
        free_http_response(response);
        return -1;
    }
    // On ajoute le header Location avec l'uri corrigé
    add_response_header("Location", new_path, response);
    if (send_http_response(osocket, response) == -1) {
        free_http_response(response);
        return -1;
    }
    free_http_response(response);

    return 0;
}

int send_304_response(SocketTCP *osocket) {
    return send_simple_response(osocket, NOT_MODIFIED_STATUS);
}

int send_400_response(SocketTCP *osocket) {
    return send_simple_response(osocket, BAD_REQUEST_STATUS);
}

int send_404_response(SocketTCP *osocket) {
    return send_simple_response(osocket, NOT_FOUND_STATUS);
}

int send_403_response(SocketTCP *osocket) {
    return send_simple_response(osocket, FORBIDEN_STATUS);
}

int send_408_response(SocketTCP *osocket) {
    return send_simple_response(osocket, TIMEOUT_STATUS);
}

int send_500_response(SocketTCP *osocket) {
    return send_simple_response(osocket, INTERNAL_ERROR_STATUS);
}

int send_501_response(SocketTCP *osocket) {
    return send_simple_response(osocket, NOT_IMPLEMENTED_STATUS);
}

bool is_modified_since(http_request *request, time_t mod_date) {
    // On cherche le header If-Modified-Since dans la requête
    header **pp = &(request->headers);
    while (*pp != NULL) {
        if (strcmp((*pp)->name, "If-Modified-Since") == 0) {
            // On convertit la dâte du header en time_t
            struct tm tm;
            strptime((*pp)->field, HTTP_DATE_FORMAT, &tm);
            time_t request_date = mktime(&tm);

            return (mod_date > request_date);
        }
        pp = &((*pp)->next);
    }
    return true;
}
