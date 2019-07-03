.PHONY : all clean

CC = gcc.exe

LOCAL_PLATFORM = x86


PATH_SOURCES = \\src_new\\
OBJECTS = $(addsuffix .c.o, $(addprefix obj/, \
	A7Unicode \
	A7Base \
	A7Bmp \
	A7Err \
	A7Gui \
	A7Main \
	A7Main_TestRipple \
	A7Main_TestRoot \
	A7MainOld \
	A7Node \
	A7Tex \
	))


PATH_SDK_SSL 		= $(A7_PATH_SDK)\\libressl-2.5.5-windows\\
PATH_SDK_FREETYPE 	= $(A7_PATH_SDK)\\freetype-2.9.1-windows-binaries\\
PATH_SDK_JPEG 		= $(A7_PATH_SDK)\\jpeg-9c\\


ifeq ( $(LOCAL_PLATFORM), x86 )
	PATH_DLL_SSL		= $(PATH_SDK_SSL)x86\\
	PATH_DLL_FREETYPE	= $(PATH_SDK_FREETYPE)win32\\
else
	PATH_DLL_SSL		= $(PATH_SDK_SSL)x64\\
	PATH_DLL_FREETYPE	= $(PATH_SDK_FREETYPE)win64\\
endif

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

ifeq ( $(LOCAL_PLATFORM), x86 )
	PATH_LIBS = $(PATH_LIBS_86)
	LD_LIBS = $(LD_LIBS_86)
else
	PATH_LIBS = $(PATH_LIBS_64)
	LD_LIBS = $(LD_LIBS_64)
endif

CPPFLAGS = $(addprefix -I, $(PATH_INCLUDE))
CFLAGS = -mwindows -municode -Wall -O3 -static -s
LDFLAGS = $(addprefix -L, $(PATH_LIBS)) -ljpeg -lGDI32 -lmingw32 -lws2_32 -lmsvcr120 $(LD_LIBS)


RM = DEL /S

all : obj a7.exe libcrypto-41.dll libssl-43.dll freetype.dll
	a7.exe

libtls-15.dll : $(PATH_DLL_SSL)libtls-15.dll
	COPY $^ $@
libssl-43.dll : $(PATH_DLL_SSL)libssl-43.dll
	COPY $^ $@
libcrypto-41.dll : $(PATH_DLL_SSL)libcrypto-41.dll
	COPY $^ $@
freetype.dll : $(PATH_DLL_FREETYPE)freetype.dll
	COPY $^ $@

clean :
	$(RM) *.exe
	$(RM) *.dll
	$(RM) *.o

obj :
	MD obj


a7.exe : $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LDFLAGS)

obj/%.c.o : $(PATH_SOURCES)/%.c
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<

