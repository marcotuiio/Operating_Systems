# Author: Marco Túlio Alves de Barros

EXEC = gcc
CFLAGS = -fstack-protector-all -g -Wall
PROJECT_NAME = gerenciador_memoria
OBJETCS = fifo.o lru.o memory.o main.o

$(PROJECT_NAME): $(OBJETCS)
	$(EXEC) $(CFLAGS) $(OBJETCS) -o $(PROJECT_NAME) -lm

fifo.o: fifo.c fifo.h
	$(EXEC) $(CFLAGS) -c fifo.c

lru.o: lru.c lru.h
	$(EXEC) $(CFLAGS) -c lru.c

memory.o: memory.c memory.h
	$(EXEC) $(CFLAGS) -c memory.c

clear: 
	rm -f *.o
	rm -f $(PROJECT_NAME)