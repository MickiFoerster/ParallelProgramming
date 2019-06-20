SRC=pthread_pool.c \
		../queue_pthread/queue.c \
		../debug_print.c
CFLAGS=-DDEBUG -g -O0 -I../queue_pthread -I.. -pthread

main : main.c $(SRC)
	gcc $(CFLAGS) $^
