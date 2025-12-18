CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = server.c access_code.c linked_list.c history.c
OBJS = $(SRCS:.c=.o)

all: server

server: $(OBJS)
	$(CC) $(CFLAGS) -o server $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o server

.PHONY: all clean
