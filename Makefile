http_parser = http_parser/
adresse_internet = adresse_internet/
socket_tcp = socket_tcp/
mime_type = mime_type/
dir_index = directory_index/
CC = gcc
CPPFLAGS = -D_XOPEN_SOURCE=700 -D_FORTYFY_SOURCE=2
CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings \
-Werror -Wformat=2 -Wdate-time -fstack-protector-all -fpie -ftrapv -ld -O2 -g \
-pthread -I$(adresse_internet) -I$(socket_tcp) -I$(http_parser) -I$(mime_type) -I$(dir_index)
LDFLAGS = -lm -z relro -z now -lpthread
VPATH=$(http_parser) $(adresse_internet) $(socket_tcp) $(mime_type) $(dir_index)
objects = $(http_parser)http_parser.o $(adresse_internet)adresse_internet.o $(socket_tcp)socket_tcp.o $(mime_type)mime_type.o $(dir_index)dir_index.o mohawks.o 
executable = mohawks

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) $(LDFLAGS) -o $(executable)

adresse_internet.o: adresse_internet.c adresse_internet.h
socket_tcp.o: socket_tcp.c socket_tcp.h 
mime_type.o: mime_type.c mime_type.h
dir_index.o: dir_index.c dir_index.h
http_parser.o: http_parser.c http_parser.h
mohawks.o: mohawks.c mohawks.h 

clean:
	$(RM) -r *~ *.txt *.tar.gz *.o $(http_parser)*.o $(adresse_internet)*.o $(socket_tcp)*.o $(mime_type)*.o $(dir_index)*.o $(executable) 

tar:
	tar -zcf "Mohawks.tar.gz" *

