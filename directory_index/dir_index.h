#ifndef DIR_INDEX__H
#define DIR_INDEX__H

#include "../mohawks.h"
#include "../socket_tcp/socket_tcp.h"

#define DIR_INDEX_FORMAT \
    "<tr><td><a href=\"%s%s\">%s%s</a></td><td>%s</td><td>%s</td></tr>"

#define INDEX_DATE_FORMAT "%d-%m-%Y %R"

// Indèxe le répertoire demandé par l'utilisateur si celui ci ne contient
// pas de fichier index.html
int directory_index(http_request *request, const char *path, SocketTCP *osocket);


#endif // DIR_INDEX.H