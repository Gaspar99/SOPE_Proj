CC=gcc
CFLAGS=-Wall -Wextra -Werror

HEADERS= args_handler.h dir_handler.h logs.h print_info.h signals_handler.h
OBJS= forensic.o args_handler.o dir_handler.o logs.o print_info.o signals_handler.o

%.0: %.c $(HEADERS)
		$(CC) $(CFLAGS) -c -o $@ $<
 
forensic: $(OBJS)
		gcc $(CFLAGS) -o $@ $^

clean:
		-rm -f $(OBJS)
		-rm -f forensic

