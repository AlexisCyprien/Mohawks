/*                          MIME_TYPE :
 *     Bibliothèque C permettant la récupération du type MIME d'un fichier.
 */

#ifndef MIME_TYPE__H
#define MIME_TYPE__H

#include <stdbool.h>

/*
 *     FONCTIONS
 */

// endswith : Indique si la chaîne s1 se termine par la chaîne s2
bool endswith(const char *s1, const char *s2);

// get_mime_type : Renvoie le type mime du fichier indiqué par filename
const char *get_mime_type(const char *filename);

#endif  // MIME_TYPE__H