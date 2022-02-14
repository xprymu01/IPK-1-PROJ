PROJECT=hinfosvc
CC=gcc
CFLAGS=-Wall -Wextra -g
SRC=hinfosvc.c


.PHONY: all test clean

all:
	$(CC) $(CFLAGS) -o $(PROJECT) $(SRC)

test: all
	./$(PROJECT) 12345 &
	curl http://localhost:12345/hostname
	curl http://localhost:12345/cpu-name
	curl http://localhost:12345/load

clean:
	rm -rf *o
	rm -rf $(PROJECT)