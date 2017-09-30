CFLAGS=-lreadline 
DEBUG=-g
#DEBUG=

all: shell


shell:	shell.c 
	gcc  $(DEBUG) shell.c -o shell $(CFLAGS) 
clean:
	rm -f shell *~

