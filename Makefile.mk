# Windows

BIN = sisop_t2
SRC = main.c memory_linkedlist.c memory_tree.c
OBJ = $(SRC:.c=.o)
CFLAGS =
DBGFLAGS = -g -Wall -Werror
LDFLAGS = 
CC = gcc

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(DBGFLAGS)

clean:
	-@ del $(OBJ) $(BIN)