
CC=gcc

CFLAGS=-Wall -g -pedantic

all: gobackRecv gobackSend

gobackRecv: gobackRecv.c
	$(CC) $(CFLAGS) gobackRecv.c -o gobackRecv

gobackSend: gobackSend.c
	$(CC) $(CFLAGS) gobackSend.c -o gobackSend

#Cleanup.
clean:
	rm -f *.o gobackRecv
	rm -f *.o gobackSend
	
