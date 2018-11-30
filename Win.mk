.PHONY : all clean

CC = gcc.exe

all : a7.exe
	a7.exe test string LOL))) А может нет

clean:
	DEL /S *.exe

a7.exe : src2/main.c
	$(CC) -o $@ -march=pentium4 -mwindows -municode -Wall -O3 $<
