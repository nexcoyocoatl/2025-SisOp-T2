# Linux e Windows

ifdef OS
   BIN = sisop_t2.exe
   fix_path = $(subst /,\,$1)
   RM = del /q
else
   ifeq ($(shell uname), Linux)
      BIN = sisop_t2
      fix_path = $1
      RM = rm -f
   endif
endif

SRC_DIR = src/
SRC = main.c memory_linkedlist.c memory_tree.c
OBJ = $(addprefix $(SRC_DIR),$(SRC:.c=.o))
CFLAGS =
DBGFLAGS = -Wall -Werror #-g
LDFLAGS =
CC = gcc

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) $(DBGFLAGS) -o $(BIN)

clean:
	-@ $(RM) $(call fix_path,$(OBJ)) $(BIN)