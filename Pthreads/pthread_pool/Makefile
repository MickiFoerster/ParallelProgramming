SRC=pthread_pool.c \
		../queue/queue.c \
		../debug_print.c ../debug_print.h
CFLAGS=-DDEBUG -g -O0 -I../queue -I.. -pthread

main : main.c $(SRC)
	gcc $(CFLAGS) $^
