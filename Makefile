#############################################################################

CC = gcc
PROJ = ipk2
LOGIN = xrisam00
ARCHIVER = tar

#############################################################################

all: client server


client: 
	$(CC) -o client client.c
     	
server: 
	$(CC) -o server server.c
     	
clean:
	-rm -f *.o *.obj $(PROJ) 
	
pack:
	-$(ARCHIVER) -zcpf $(LOGIN).tar.gz *.pdf *.c *.h Makefile

#############################################################################
