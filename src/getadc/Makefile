NAME=getadc
SRCS=getadc.c
CFLAGS=-fno-tree-cselim -Wall -O0 -mcpu=arm9 -o $(NAME)
CC=gcc
INSTALL=/usr/local/bin/

all: main install

main: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

install:
	cp $(NAME) $(INSTALL)$(NAME)
	chown root:root $(INSTALL)$(NAME)
	chmod 4755 $(INSTALL)$(NAME)

clean:
	/bin/rm -f $(NAME)
