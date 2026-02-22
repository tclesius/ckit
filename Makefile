CC = cc
CFLAGS = -Wall -I. -framework Cocoa

example:
	$(CC) $(CFLAGS) examples/$(NAME).c -o examples/$(NAME).o && ./examples/$(NAME).o
