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

#include "../mohawks.h"
#include "../socket_tcp/socket_tcp.h"

int directory_index(http_request *request, const char *path, SocketTCP *osocket) {
    request = request;
    osocket = osocket;
    DIR *d;
    struct dirent *dir;
    char dirpath[PATH_MAX];
    snprintf(dirpath, strlen(path)-strlen(DEFAULT_INDEX), "%s", path);
    d = opendir(dirpath);

    char body[HTTP_RESP_SIZE];
    memset(body, 0, sizeof(body));

    int head_fd;
    if ((head_fd = open("./directory_index/header.html", O_RDONLY)) == -1) {
        return -1;
    }
    if (read(head_fd, body, sizeof(body)) == -1) {
        return -1;
    }
    close(head_fd);


    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 &&
                strcmp(dir->d_name, "..") != 0) {
                    char new_tr[256];

                    char filepath[PATH_MAX*2];
                    snprintf(filepath, sizeof(filepath) -1, "%s/%s", dirpath, dir->d_name);
                    struct stat filestat;
                    if (stat(filepath, &filestat) == -1) {
                        perror("stat");
                        closeSocketTCP(osocket);
                        return -1;
                    }

                    struct tm tm;
                    localtime_r(&(filestat.st_mtim.tv_sec), &tm);
                    char date[200];
                    strftime(date, sizeof(date) -1, "%d-%m-%Y %R", &tm);

                    if (dir->d_type == DT_DIR) {
                        snprintf(new_tr, sizeof(new_tr), DIR_INDEX_FORMAT, 
                                    dir->d_name, 
                                    "/",
                                    dir->d_name,
                                    "/",
                                    date,
                                    (unsigned long) filestat.st_size);
                    } else {
                        snprintf(new_tr, sizeof(new_tr), DIR_INDEX_FORMAT, 
                                dir->d_name, 
                                "", 
                                dir->d_name, 
                                "",
                                date,
                                (unsigned long) filestat.st_size);
                    }
                    strncat(body, new_tr, sizeof(body) - 1);
            }
        }
        closedir(d);
    }
    
    int foot_fd;
    if ((foot_fd = open("./directory_index/footer.html", O_RDONLY)) == -1) {
        return -1;
    }
    if (read(foot_fd, body + strlen(body), sizeof(body)) == -1) {
        return -1;
    }
    close(foot_fd);

    http_response *response = malloc(sizeof(http_response));
    if (response == NULL) {
        return -1;
    }
    if (create_http_response(response, HTTP_VERSION, OK_STATUS, body, sizeof(body)) == -1) {
        closeSocketTCP(osocket);
        return -1;
    }
    if (send_http_response(osocket, response) == -1) {
        free_http_response(response);
        closeSocketTCP(osocket);
        return -1;
    }
    free_http_response(response);

    return(0);
}


