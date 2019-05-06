CC=gcc
CFLAGS=-Wall -Wextra -Werror -D_REENTRANT -lpthread

HEADERS= sope.h types.h constants.h
OBJS_SRV= log.c server.c
OBJS_USR= log.c user.c

%.0: %.c $(HEADERS)
		$(CC) $(CFLAGS) -c -o $@ $<
 
server: $(OBJS_SRV)
		gcc $(CFLAGS) -o $@ $^

user: $(OBJS_USR)		
		gcc $(CFLAGS) -o $@ $^

clean:
		-rm -f $(OBJS_SRV)
		-rm -f server
		-rm -f $(OBJS_USR)
		-rm -f user

