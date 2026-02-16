CC := gcc
CFLAGS := -Wall -Wextra
OBJ_FLAG := -c
DEBUG  := -g
INCL_DIR := -Iinclude

SRC_DIR := src

all: src/ast.c src/command.c src/list.c src/parser.c src/pipeline.c src/utils.c src/lexer/*.c src/main.c src/shell.c src/io/input.c
	$(CC) $(CFLAGS) $(INCL_DIR) $^

debug: src/ast.c src/command.c src/list.c src/parser.c src/pipeline.c src/utils.c src/lexer/*.c src/main.c src/shell.c src/io/input.c
	$(CC) $(CFLAGS) $(DEBUG) $(INCL_DIR) $^

clean:
	rm  *.o a.out