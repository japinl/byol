CC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -ledit
BIN = prompt parsing

.PHONY: all

all: $(BIN)

prompt: prompt.o
	$(CC) -o $@ $^ $(LIBS)
parsing: parsing.o mpc.o
	$(CC) -o $@ $^ $(LIBS)

*.o: *.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm *.o $(BIN)
