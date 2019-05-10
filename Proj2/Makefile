CC=gcc
CFLAGS=-Wall -Wextra -Werror -D_REENTRANT -pthread

HEADERS= sope.h types.h constants.h error_checker.h tlv_request_filler.h bank_office.h requests_queue.h
OBJS_SRV= log.c server.c bank_office.c requests_queue.c
OBJS_USR= log.c user.c error_checker.c tlv_request_filler.c

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

