# Author: Marco Túlio Alves de Barros

EXEC = gcc
CFLAGS = -fstack-protector-all -g -Wall
PROJECT_NAME = gerenciador_memoria
OBJETCS = memory.o main.o

$(PROJECT_NAME): $(OBJETCS)
	$(EXEC) $(CFLAGS) $(OBJETCS) -o $(PROJECT_NAME) -lm

memory.o: memory.c memory.h
	$(EXEC) $(CFLAGS) -c memory.c

clear: 
	rm -f *.o
	rm -f $(PROJECT_NAME)
	rm -f a.out