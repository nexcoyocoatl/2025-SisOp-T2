# Windows

BIN = sisop_t2
SRC = main.c memory.c memory_linkedlist.c
DEP = # single header files
OBJ = $(SRC:.c=.o)
CFLAGS =
DBGFLAGS = -g -Wall -Werror
LDFLAGS = 
CC = gcc

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(DEP) -o $@ $(LDFLAGS)

clean:
	-@ del $(OBJ) $(BIN)