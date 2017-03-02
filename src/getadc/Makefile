NAME=getadc
SRCS=getadc.c
CFLAGS=-fno-tree-cselim -Wall -O0 -mcpu=arm9 -o $(NAME)
CC=gcc

all: main install

main: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

install:
	cp $(NAME) /usr/local/bin/$(NAME)

clean:
	/bin/rm -f $(NAME)
