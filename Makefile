CC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -ledit

.PHONY: all

all: prompt

prompt: prompt.o
	$(CC) -o $@ $^ $(LIBS)

prompt.o: prompt.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm *.o prompt
