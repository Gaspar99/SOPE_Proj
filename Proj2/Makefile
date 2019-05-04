CC=gcc
CFLAGS=-Wall -Wextra -Werror -D_REENTRANT -lpthread

HEADERS= 
OBJS= server.o

%.0: %.c $(HEADERS)
		$(CC) $(CFLAGS) -c -o $@ $<
 
server: $(OBJS)
		gcc $(CFLAGS) -o $@ $^

clean:
		-rm -f $(OBJS)
		-rm -f server

