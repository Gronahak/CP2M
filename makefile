CC = gcc -Wall -O3

organeDePresse : initial.o types.h journaliste archiviste
	$(CC) initial.o -o organeDePresse

initial.o : initial.c types.h

archiviste : archiviste.o types.h

archiviste.o : archiviste.c types.h

journaliste : journaliste.o types.h

journaliste.o : journaliste.c types.h


propre:
	rm *.o organeDePresse journaliste archiviste


