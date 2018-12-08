.PHONY : all clean

CC = gcc.exe

PATH_SDK_SSL 		= F:\\sdk\\libressl-2.5.5-windows\\
PATH_SDK_FREETYPE 	= F:\\sdk\\freetype-2.9.1-windows-binaries\\
PATH_SDK_JPEG 		= F:\\sdk\\jpeg-9c\\

PATH_INCLUDE =\
	$(PATH_SDK_SSL)include\
	$(PATH_SDK_FREETYPE)include\
	$(PATH_SDK_JPEG)

PATH_LIBS = \
	$(PATH_SDK_SSL)x86\
	$(PATH_SDK_FREETYPE)win32\
	$(PATH_SDK_JPEG)

LD_LIBS =\
	$(addprefix $(PATH_SDK_SSL)x86\\, libcrypto-41.lib libssl-43.lib )\
	$(addprefix $(PATH_SDK_FREETYPE)win32\\, freetype.lib )

CPPFLAGS = $(addprefix -I, $(PATH_INCLUDE))
CFLAGS = -mwindows -municode -march=pentium4 -Wall -O3
LDFLAGS = $(addprefix -L, $(PATH_LIBS)) -ljpeg -lGDI32 -lmingw32 -lws2_32 $(LD_LIBS)
OBJECTS =\
	obj/main.c.o

all : a7.exe
	a7.exe

clean:
	DEL /S *.exe
	DEL /S *.o


a7.exe : $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LDFLAGS)

obj/%.c.o : ss/%.c
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<
