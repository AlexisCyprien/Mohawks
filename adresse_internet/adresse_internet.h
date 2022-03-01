#ifndef ADRESSE_INTERNET_H_
#define ADRESSE_INTERNET_H_

#include <stdint.h>
#include <netinet/ip.h> // Pour in_port_t
#define _XOPEN_SOURCE 700

#include "adresse_internet_type.h"

/**
 * construit une adresse internet à partir d’un éventuel nom (sous
 * forme DNS ou IP) et d’un numéro de port. L’argument adresse est un
 * pointeur vers une variable de type adresse_internet, qui est remplie
 * avec l’adresse internet construite et allouée. Valeur de retour :
 * en cas de succès, la fonction renvoie le pointeur. En cas d’erreur
 * la fonction renvoie NULL.
 */
extern adresse_internet *adresse_internet_new(const char *nom, uint16_t port);
/**
 * Idem, mais construit une adresse internet correspondant à toutes
 * les interfaces réseau à partir d’un numéro de port.
 */
extern adresse_internet *adresse_internet_any(uint16_t port);
/**
 * Idem, mais construit une adresse internet correspondant à
 * l'interface loopback à partir d’un numéro de port.
 */
extern adresse_internet *adresse_internet_loopback(uint16_t port);
/**
 * libère la mémoire allouée par adresse_internet_free, any et loopback
 */
extern void adresse_internet_free(adresse_internet *adresse);

/**
 * extrait d’une adresse internet l’adresse IP et le port
 * correspondants. L’argument adresse pointe vers un buffer
 * contenant une adresse. L’argument nom_dns (resp. nom_port)
 * pointe vers un buffer alloué de taille au moins taille_dns
 * (resp. taille_port) dans lequel la fonction va écrire une chaîne
 * de caractère (terminant par un 0) contenant le nom (resp. le port)
 * associé à l’adresse fournie. Lorsque cela est possible, la
 * résolution de nom est faite.  Si nom_dns ou nom_port est NULL,
 * l’extraction correspondante ne sera pas effectuée. Les deux ne
 * peuvent pas être NULL en même temps.  Valeur de retour : rend 0 en
 * cas de succès, et -1 en cas d’erreur.
 */
extern int adresse_internet_get_info(adresse_internet *adresse,
    char *nom_dns, int taille_dns, char *nom_port, int taille_port);

extern int adresse_internet_get_ip(const adresse_internet *adresse, char *ip,
    int taille_ip);
/**
 * extrait le numéro de port d’une adresse internet. L'argument
 * adresse se comporte comme dans la fonction
 * précédente.  Valeur de retour : renvoie le port en cas de succès,
 * et 0 en cas d’erreur (par exemple si adresse n’est pas
 * initialisée).
 */
extern uint16_t adresse_internet_get_port(const adresse_internet *adresse);
/**
 * rend le domaine internet de l'adresse (AF_INET ou AF_INET6) ou -1
 * si l'adresse n'est pas initialisée.
 */

extern int adresse_internet_get_domain(const adresse_internet *adresse);
/**
 * Construit une adresse internet à partir d'une structure sockaddr La
 * structure addresse doit être suffisamment grande pour pouvoir
 * accueillir l’adresse.  Valeur de retour : 0 en cas de succès, -1 en
 * cas d’échec.
 */

extern int sockaddr_to_adresse_internet(const struct sockaddr *addr,
    adresse_internet *adresse);
/**
 * Construit une structure sockaddr à partir d'une adresse
 * internet. La structure addr doit être suffisamment grande pour
 * pouvoir accueillir l’adresse.  Valeur de retour : 0 en cas de
 * succès, -1 en cas d’échec.
 */
extern int adresse_internet_to_sockaddr(const adresse_internet *adresse,
    struct sockaddr *addr);
/**
 * compare deux adresse internet adresse1 et adresse2.  Valeur de
 * retour : rend 0 si les adresses sont différentes, 1 si elles sont
 * identiques (même IP et même port), et -1 en cas d’erreur.
 *
 */
extern int adresse_internet_compare(const adresse_internet *adresse11,
    const adresse_internet *adresse22);

/* copie une adresse internet dans une autre. Les variables doivent
 * être allouées. */
extern int adresse_internet_copy(adresse_internet *adrdst,
    const adresse_internet *adrsrc);

#endif // ADRESSE_INTERNET_H_
