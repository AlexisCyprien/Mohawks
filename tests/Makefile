http_parser = ../http_parser/
CC = gcc
CPPFLAGS = -D_XOPEN_SOURCE=700 -D_FORTYFY_SOURCE=2
CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings \
-Werror -Wformat=2 -Wdate-time -fstack-protector-all -fpie -ftrapv -ld -O2 -g 
#LDLIBS = -pthread -Wl,-z,relro,-z,now -fpie -lm 
LDFLAGS = -lm -z relro -z now
VPATH=$(http_parser)
objects = $(http_parser)http_parser.o unit_test.o
executable = unit_test

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) $(LDFLAGS) -o $(executable)

http_parser.o: http_parser.c http_parser.h
unit_test.o: unit_test.c unit_test.h http_parser.h

clean:
	$(RM) -r *~ *.o $(http_parser)*.o $(executable)



