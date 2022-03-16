#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "dir_index.h"

#include <dirent.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#include "../mohawks.h"
#include "../socket_tcp/socket_tcp.h"

int directory_index(http_request *request, const char *path, SocketTCP *osocket) {
    // On récupère le chemin du dossier demandé
    char dirpath[PATH_MAX];
    snprintf(dirpath, strlen(path)-strlen(DEFAULT_INDEX), "%s", path);
    
    // On créé notre buffer de réponse
    char body[HTTP_RESP_SIZE];
    memset(body, 0, sizeof(body));

    // On ouvre notre header html
    int head_fd;
    if ((head_fd = open("./directory_index/header.html", O_RDONLY)) == -1) {
        return send_500_response(osocket);
    }
    // On ajoute son contenu au corps de la réponse
    if (read(head_fd, body, sizeof(body)) == -1) {
        return send_500_response(osocket);
    }
    if (close(head_fd) == -1){
        return send_500_response(osocket);
    }

    // Pour des raisons de sécurité, la gestion du bouton de retour dans le
    // dossier précédent se fait côté serveur et non côté client.
    // Le retour dans un dossier en dessous de DEFAULT_CONTENT_DIR est de toute
    // façon impossible sur notre serveur.
    if (strcmp(request->request_line->uri, "/") != 0) {
        const char *prev_dir = "<tr><td><a id=\"prev_dir\" href=\"../\">&larrhk;</a></td></tr>"; 
        strncat(body, prev_dir, sizeof(body) - 1);
    }

    // On ouvre le dossier
    DIR *d;
    d = opendir(dirpath);
    if (d == NULL) {
        switch (errno) {
        case EACCES:
            // Pas autorisé à accéder à ce dossier
            return send_403_response(osocket);
            break;
        case ENOENT :
        case ENOTDIR :
            // Le dossier n'existe pas ou n'est pas un dossier
            return send_404_response(osocket);
            break;
        case EMFILE :
        case ENFILE :
        case ENOMEM :
            // Nb de descripteurs de fichiers maximum atteint ou
            // plus de mémoire disponnible
            return send_500_response(osocket);
            break;
        default:
            break;
        }
    }

    // On lit toutes les entrées de notre dossier
    struct dirent *dir;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // Si l'entrée est un lien sur ce dossier ou le précédent,
            // nous ne faisons rien.
            if (strcmp(dir->d_name, ".") != 0 &&
                strcmp(dir->d_name, "..") != 0) {
                    // On initialise notre nouvelle ligne du tableau
                    char new_tr[strlen(DIR_INDEX_FORMAT) + PATH_MAX*2 + 202];

                    // On construit le chemin juqu'au dossier pour la fonction stat
                    char filepath[PATH_MAX*2];
                    snprintf(filepath, sizeof(filepath) -1, "%s/%s", dirpath, dir->d_name);
                    struct stat filestat;
                    // On récupère les informations sur cette entrée
                    if (stat(filepath, &filestat) == -1) {
                        return -1;
                    }
                    // On transforme la dernière dâte de modification en
                    // chaîne de caractère 
                    struct tm tm;
                    localtime_r(&(filestat.st_mtim.tv_sec), &tm);
                    char date[200];
                    strftime(date, sizeof(date) -1, INDEX_DATE_FORMAT, &tm);

                    // L'entrée est un dossier
                    if (dir->d_type == DT_DIR) {
                        // On remplie la nouvelle ligne du tableau
                        snprintf(new_tr, sizeof(new_tr), DIR_INDEX_FORMAT, 
                                    dir->d_name, 
                                    "/",
                                    dir->d_name,
                                    "/",
                                    date,
                                    "-");

                    // Sinon, c'est un fichier
                    } else {
                        // On transforme sa taille en chaîne de caractère
                        char size[sizeof(unsigned long) + 1];
                        snprintf(size, sizeof(size), "%ld", (unsigned long) filestat.st_size);

                        // On remplie la nouvelle ligne du tableau
                        snprintf(new_tr, sizeof(new_tr), DIR_INDEX_FORMAT, 
                                dir->d_name, 
                                "", 
                                dir->d_name, 
                                "",
                                date,
                                size);
                    }
                    // On ajoute cette ligne au corps de la réponse
                    strncat(body, new_tr, sizeof(body) - 1);
            }
        }
        if (closedir(d) == -1) {
            return send_500_response(osocket);
        }
    }
    
    // On ouvre notre footer html
    int foot_fd;
    if ((foot_fd = open("./directory_index/footer.html", O_RDONLY)) == -1) {
        return send_500_response(osocket);
    }
    // On ajoute son contenu à la suite de notre corps de réponse
    if (read(foot_fd, body + strlen(body), sizeof(body)) == -1) {
        return send_500_response(osocket);
    }
    if (close(foot_fd) == -1) {
        return send_500_response(osocket);
    }

    http_response *response = malloc(sizeof(http_response));
    if (response == NULL) {
        return -1;
    }
    // On créé notre réponse
    if (create_http_response(response, HTTP_VERSION, OK_STATUS, body, strlen(body)) == -1) {
        return -1;
    }
    // On ajoute le header Content-Type
    add_response_header("Content-Type", "text/html", response);
    
    // On envoie notre répons au client
    if (send_200_response(osocket, response) == -1) {
        free_http_response(response);
        return -1;
    }
    free_http_response(response);

    return(0);
}


