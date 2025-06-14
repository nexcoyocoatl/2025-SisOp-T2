# Windows

BIN = sisop_t2
SRC = main.c memory.c memory_linkedlist.c
OBJ = $(SRC:.c=.o)
CFLAGS =
DBGFLAGS = -g -Wall -Werror
LDFLAGS = 
CC = gcc

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(DBGFLAGS)

clean:
	-@ del $(OBJ) $(BIN)