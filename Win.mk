.PHONY : all clean

CC = gcc.exe

PATH_SDK_SSL 		= $(a7_path_sdk)\\libressl-2.5.5-windows\\
PATH_SDK_FREETYPE 	= $(a7_path_sdk)\\freetype-2.9.1-windows-binaries\\
PATH_SDK_JPEG 		= $(a7_path_sdk)\\jpeg-9c\\

PATH_DLL_SSL 		= $(PATH_SDK_SSL)x64\\
PATH_DLL_FREETYPE	= $(PATH_SDK_FREETYPE)win64\\

PATH_INCLUDE =\
	$(PATH_SDK_SSL)include\
	$(PATH_SDK_FREETYPE)include\
	$(PATH_SDK_JPEG)

PATH_LIBS_86 = \
	$(PATH_SDK_SSL)x86\
	$(PATH_SDK_FREETYPE)win32\
	$(PATH_SDK_JPEG)

LD_LIBS_86 =\
	$(addprefix $(PATH_SDK_SSL)x86\\, libcrypto-41.lib libssl-43.lib )\
	$(addprefix $(PATH_SDK_FREETYPE)win32\\, freetype.lib )

PATH_LIBS_64 = \
	$(PATH_SDK_SSL)x64\
	$(PATH_SDK_FREETYPE)win64\
	$(PATH_SDK_JPEG)

LD_LIBS_64 =\
	$(addprefix $(PATH_SDK_SSL)x64\\, libcrypto-41.lib libssl-43.lib )\
	$(addprefix $(PATH_SDK_FREETYPE)win64\\, freetype.lib )

PATH_LIBS = $(PATH_LIBS_64)
LD_LIBS = $(LD_LIBS_64)

CPPFLAGS = $(addprefix -I, $(PATH_INCLUDE)) 
CFLAGS = -mwindows -municode -Wall -O3 -static -s
LDFLAGS = $(addprefix -L, $(PATH_LIBS)) -ljpeg -lGDI32 -lmingw32 -lws2_32 -lmsvcr120 $(LD_LIBS)
OBJECTS =\
	obj/main.c.o

RM = DEL /S

all : a7.exe libcrypto-41.dll libssl-43.dll freetype.dll
	a7.exe

libtls-15.dll : $(PATH_DLL_SSL)libtls-15.dll
	COPY $^ $@
libssl-43.dll : $(PATH_DLL_SSL)libssl-43.dll
	COPY $^ $@
libcrypto-41.dll : $(PATH_DLL_SSL)libcrypto-41.dll
	COPY $^ $@
freetype.dll : $(PATH_DLL_FREETYPE)freetype.dll
	COPY $^ $@

clean:
	$(RM) *.exe
	$(RM) *.dll
	$(RM) *.o


a7.exe : $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LDFLAGS)

obj/%.c.o : ss/%.c
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<
