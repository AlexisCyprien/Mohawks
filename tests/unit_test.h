#ifndef UNIT_TEST__H
#define UNIT_TEST__H

#define GOOD_REQUEST_LINE "GET /index.html HTTP/1.0"
#define BAD_REQUEST_LINE "DONT GET /ind.pdf DHCP/1.0"

// test_parse_request_line : Teste le bon fonctionnement de la fonction
//      parse_request_line dans http_parser
int test_parse_request_line(const char *rawdata);

const char *err_to_string(int err);

#endif  // UNIT_TEST__H