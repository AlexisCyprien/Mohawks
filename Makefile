http_parser = http_parser/
adresse_internet = adresse_internet/
socket_tcp = socket_tcp/
CC = gcc
CPPFLAGS = -D_XOPEN_SOURCE=700 -D_FORTYFY_SOURCE=2
CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings \
-Werror -Wformat=2 -Wdate-time -fstack-protector-all -fpie -ftrapv -ld -O2 -g \
-pthread -I$(adresse_internet) -I$(socket_tcp) -I$(http_parser)
LDFLAGS = -lm -z relro -z now -lpthread
VPATH=$(http_parser) $(adresse_internet) $(socket_tcp)
objects = $(http_parser)http_parser.o $(adresse_internet)adresse_internet.o $(socket_tcp)socket_tcp.o http_server.o
executable = http_server

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) $(LDFLAGS) -o $(executable)

adresse_internet.o: adresse_internet.c adresse_internet.h
socket_tcp.o: socket_tcp.c socket_tcp.h 
http_parser.o: http_parser.c http_parser.h
http_server.o: http_server.c http_server.h 

clean:
	$(RM) -r *~ *.o $(http_parser)*.o $(executable)



