NAME=getadc
SRCS=getadc.c
CFLAGS=-fno-tree-cselim -Wall -O0 -mcpu=arm9 -o $(NAME)
CC=gcc
INSTALL=/usr/local/bin/

all: main install

main: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)
	chown root:root $(NAME)
	chmod 4755 $(NAME)

install:
	cp $(NAME) $(INSTALL)$(NAME)

clean:
	/bin/rm -f $(NAME)
	/bin/rm -f $(INSTALL)$(NAME)
