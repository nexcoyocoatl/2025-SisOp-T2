# Linux e Windows

SRC_DIR = src
SRC = main.c memory_linkedlist.c memory_tree.c

ifdef OS
   BIN = sisop_t2.exe
   PATH_SLASH = \\
   RM = del /q
else
   BIN = sisop_t2
   ifeq ($(shell uname), Linux)
      PATH_SLASH = /
      RM = rm -f
   endif
endif

SRC_PATH = $(addprefix $(SRC_DIR),$(PATH_SLASH))
OBJ = $(addprefix $(SRC_PATH),$(SRC:.c=.o))
CFLAGS =
DBGFLAGS = -Wall -Werror #-g
LDFLAGS =
CC = gcc

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) $(DBGFLAGS) -o $(BIN)

clean:
	-@ $(RM) $(OBJ) $(BIN)