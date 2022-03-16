#ifndef DIR_INDEX__H
#define DIR_INDEX__H

#include "../mohawks.h"
#include "../socket_tcp/socket_tcp.h"

#define DIR_INDEX_FORMAT \
    "<tr><td><a href=\"%.20s%s\">%.20s%s</a></td><td>%.20s</td><td>%ld</td></tr>\r\n"

// Indexe le répertoire demandé par l'utilisateur si celui ci ne contient
// pas de fichier index.html
int directory_index(http_request *request, const char *path, SocketTCP *osocket);


#endif // DIR_INDEX.H