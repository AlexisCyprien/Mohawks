#ifndef UNIT_TEST__H
#define UNIT_TEST__H

#define GOOD_REQUEST_LINE "GET /index.html HTTP/1.0"
#define BAD_REQUEST_LINE "DONT GET /ind.pdf DHCP/1.0"

#define GOOD_HEADER "Content-Type: text/html"
#define BAD_HEADER "PAS UN HEADER ! ! !"

#define GOOD_HTTP_REQUEST(RL, HD) RL "\r\n" HD
#define BAD_HTTP_REQUEST(RL, HD) RL "\r\n" HD

// test_parse_request_line : Teste le bon fonctionnement de la fonction
//      parse_request_line dans http_parser
int test_parse_request_line(const char *rawdata);

// test_parse_header_line : Teste le bon fonctionnement de la fonction
//      parse_header_line dans http_parser
int test_parse_header(const char *rawdata);

// test_parse_http_request : Teste le bon fonctionnement de la fonction
//      parse_http_request dans http_parser
int test_parse_http_request(const char *rawdata);

// err_to_string : Renvoie à partir du code d'erreur la chaine de caractères
//      qui lui correspond
const char *err_to_string(int err);

#endif  // UNIT_TEST__H