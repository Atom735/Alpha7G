.PHONY : all clean

SDK_PATH_SDL = F:\\sdk\\SDL2-2.0.8\\i686-w64-mingw32\\
SDK_PATH_SSL = F:\\sdk\\libressl-2.5.5-windows\\
SDK_PATH_FREETYPE = F:\\sdk\\freetype-2.9.1-windows-binaries\\
SDK_PATH_JPEG = F:\\sdk\\jpeg-9c\\


# SDK_PATH_SSL2 = F:\\sdk\\libressl-2.8.2\\

PATH_INCLUDE =\
	$(SDK_PATH_SDL)include\
	$(SDK_PATH_SSL)include\
	$(SDK_PATH_FREETYPE)include\
	$(SDK_PATH_JPEG)

PATH_LIBS = \
	$(SDK_PATH_SDL)lib\
	$(SDK_PATH_SSL)x86\
	$(SDK_PATH_FREETYPE)win32\
	$(SDK_PATH_JPEG)

PRE_DEFINES = WIN32_LEAN_AND_MEAN _WIN32_WINNT=_WIN32_WINNT_WIN7
PRE_LIBS =\
	$(addprefix $(SDK_PATH_SSL)x86\\, libcrypto-41.lib libssl-43.lib )\
	$(addprefix $(SDK_PATH_FREETYPE)win32\\, freetype.lib )

CC = gcc.exe

CPPFLAGS = $(addprefix -D, $(PRE_DEFINES)) $(addprefix -I, $(PATH_INCLUDE))
CFLAGS = -mwindows -municode -march=pentium4 -Wall -O3
LDFLAGS = $(addprefix -L, $(PATH_LIBS)) -ljpeg -lGDI32 -lmingw32 -lws2_32 $(PRE_LIBS)

FILES = a7main.c a7log.c a7err.c a7net.c a7ut.c
SOURCES = $(addprefix src/, $(FILES))
OBJECTS = $(addsuffix .o, $(addprefix obj/, $(FILES)))

TESTS =\
	gdi\
	gdi_jpg\
	net_tls_getfile\
	gdi_freetype

T_TESTS = $(addprefix test/, $(addsuffix .exe, $(TESTS)))

all : main.exe k.exe $(T_TESTS)
	test/gdi_freetype.exe

clean:
	DEL /S *.dll
	DEL /S *.exe
	DEL /S *.log
	DEL /S *.a7-log
	DEL /S *.o

test/%.exe : test/%.c
	$(CC) -o $@ $(CPPFLAGS) -march=pentium4 -Wall -O3 -g $< $(LDFLAGS)

# libssl-43.dll libtls-15.dll
main.exe : $(OBJECTS) libcrypto-41.dll libssl-43.dll freetype.dll
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LDFLAGS)

obj/%.c.o : src/%.c
	$(CC) -o $@ -c $(CPPFLAGS) $(CFLAGS) $<



SDL2.dll : $(SDK_PATH_SDL)bin\\SDL2.dll
	COPY $^ $@
libtls-15.dll : $(SDK_PATH_SSL)x86\\libtls-15.dll
	COPY $^ $@
libssl-43.dll : $(SDK_PATH_SSL)x86\\libssl-43.dll
	COPY $^ $@
libcrypto-41.dll : $(SDK_PATH_SSL)x86\\libcrypto-41.dll
	COPY $^ $@
freetype.dll : $(SDK_PATH_FREETYPE)win32\\freetype.dll
	COPY $^ $@

k.exe : src/main.c
	$(CC) -o $@ $(CPPFLAGS) -march=pentium4 -Wall -O3 -g $< -lmingw32 -lws2_32 $(PRE_LIBS)



# AR
# Программа работы с архивами; по умолчанию, `ar'.
# AS
# Ассемблер; по умолчанию, `as'.
# CC
# Компилятор Си; по умолчанию, `cc'.
# CXX
# Компилятор C++; по умолчанию, `g++'.
# CO
# Программа для извлечения файлов из RCS; по умолчанию, `co'.
# CPP
# Препроцессор языка Си, выдающий результат на стандартный вывод; по умолчанию, `$(CC) -E'.
# FC
# Препроцессор и компилятор для Фортрана и Ратфора; по умолчанию, `f77'.
# GET
# Программа для извлечения файлов из SCCS; по умолчанию, `get'.
# LEX
# Программа для преобразования Lex-грамматики в текст на языках Си или Ратфор; по умолчанию - `lex'.
# PC
# Компилятор Паскаля; по умолчанию, `pc'.
# YACC
# Программа для преобразования Yacc-грамматики в текст на Си; по умолчанию - `yacc'.
# YACCR
# Программа для преобразования Yacc-грамматики в текст на языке Ратфор; по умолчанию - `yacc -r'.
# MAKEINFO
# Программа для преобразования исходного файла формата Texinfo в файл формата Info; по умолчанию, `makeinfo'.
# TEX
# Программа для преобразования исходных файлов на TeX в файлы формата DVI; по умолчанию - `tex'.
# TEXI2DVI
# Программа, преобразующая исходные файлы в формате Texinfo, в DVI-файлы программы TeX; по умолчанию - `texi2dvi'.
# WEAVE
# Программа, преобразующая текст из формата Web в формат TeX; по умолчанию - `weave'.
# CWEAVE
# Программа, преобразующая текст на Си-Web в формат TeX; по умолчанию - `cweave'.
# TANGLE
# Программа, преобразующая текст на Web в Паскаль; по умолчанию - `tangle'.
# CTANGLE
# Программа, преобразующая текст на Си-Web в текст на Си; по умолчанию - `ctangle'.
# RM
# Команда удаления файла; по умолчанию, `rm -f'.
# Ниже приведена таблица переменных, содержащих дополнительные параметры для перечисленных выше программ. По умолчанию, значением этих переменных является пустая строка (если не указано другое).
#
# ARFLAGS
# Опции, передаваемые программе, манипулирующей с архивными файлам; по умолчанию `rv'.
# ASFLAGS
# Дополнительные параметры, передаваемые ассемблеру (при его явном вызове для файлов `.s' и `.S').
# CFLAGS
# Дополнительные параметры, передаваемые компилятору Си.
# CXXFLAGS
# Дополнительные параметры, передаваемые компилятору C++.
# COFLAGS
# Дополнительные параметры, передаваемые программе co (входящей в систему RCS).
# CPPFLAGS
# Дополнительные параметры, передаваемые препроцессору языка Си и программам, его использующим (компиляторам Си и Фортрана).
# FFLAGS
# Дополнительные параметры для компилятора Фортрана.
# GFLAGS
# Дополнительные параметры, передаваемые программе get (входящей в систему SCCS).
# LDFLAGS
# Дополнительные параметры, передаваемые компиляторам, когда предполагается вызов компоновщика `ld'.
# LFLAGS
# Дополнительные параметры, передаваемые программе Lex.
# PFLAGS
# Дополнительные параметры, передаваемые компилятору Паскаля.
# RFLAGS
# Дополнительные параметры, передаваемые компилятору Фортрана при компиляции программ на Ратфоре.
# YFLAGS
# Дополнительные параметры, передаваемые программе Yacc.
