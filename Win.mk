.PHONY : all clean

CC = gcc.exe


CPPFLAGS =
CFLAGS = -mwindows -municode -march=pentium4 -Wall -O3
LDFLAGS =
OBJECTS =\
	obj/funcstack.c.o\
	obj/log.c.o\
	obj/main.c.o

all : a7.exe
	a7.exe

clean:
	DEL /S *.exe
	DEL /S *.o


a7.exe : $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LDFLAGS)

obj/%.c.o : src2/%.c
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<
