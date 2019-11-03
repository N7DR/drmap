# makefile for drlog

CC = ccache g++
CPP = /usr/local/bin/cpp
LIB = ar
ASM = as

# The places to look for include files (in order).
INCL =  -I./include -I/usr/share/R/include -I/usr/lib/R/site-library/Rcpp/include -I/usr/lib/R/site-library/RInside/include

# LIBS = -L

LIBRARIES = -lR -lRInside -lstdc++fs

LIBINCL = -L. -L/usr/lib/R/lib -L/usr/lib/R/site-library/RInside/lib

# Extra Defs

# utility routines
DEL = rm
COPY = cp

# name of main executable to build
PROG = all

# -O works
# -O1 works
# -O2 works
CFLAGS = $(INCL) -D_REENTRANT -c -g3 -O2 -pipe -DLINUX -D_FILE_OFFSET_BITS=64 -fmessage-length=0 -Wno-reorder -fconcepts -std=c++17

LINKFLAGS = $(LIBINCL) -Wl,--export-dynamic -fopenmp -Wl,-rpath,/usr/lib/R/site-library/RInside/lib
	
# command_line.h has no dependencies
	
# diskfile.h has no dependencies

include/grid_float.h : include/string_functions.h
	touch include/grid_float.h
	
# drlog-error.h has no dependencies

# macros.h has no dependencies

include/memory.h : include/macros.h
	touch include/memory.h

include/r_figure.h : include/macros.h
	touch include/r_figure.h

include/string_functions.h : include/macros.h include/x_error.h
	touch include/string_functions.h

# x_error.h has no dependencies
	
src/command_line.cpp : include/command_line.h
	touch src/command_line.cpp
	
src/diskfile.cpp : include/diskfile.h
	touch src/diskfile.cpp
	
src/drmap.cpp : include/command_line.h include/diskfile.h include/grid_float.h include/memory.h include/r_figure.h
	touch src/drmap.cpp
	
src/grid_float.cpp : include/diskfile.h include/grid_float.h include/string_functions.h
	touch src/grid_float.cpp
	
src/memory.cpp : include/memory.h include/string_functions.h
	touch src/memory.cpp

src/r_figure.cpp : include/r_figure.h
	touch src/r_figure.cpp

src/string_functions.cpp : include/macros.h include/string_functions.h
	touch src/string_functions.cpp
	
bin/command_line.o : src/command_line.cpp
	$(CC) $(CFLAGS) -o $@ src/command_line.cpp

bin/diskfile.o : src/diskfile.cpp
	$(CC) $(CFLAGS) -o $@ src/diskfile.cpp

bin/drmap.o : src/drmap.cpp
	$(CC) $(CFLAGS) -o $@ src/drmap.cpp

bin/grid_float.o : src/grid_float.cpp
	$(CC) $(CFLAGS) -o $@ src/grid_float.cpp

bin/memory.o : src/memory.cpp
	$(CC) $(CFLAGS) -o $@ src/memory.cpp

bin/r_figure.o : src/r_figure.cpp
	$(CC) $(CFLAGS) -o $@ src/r_figure.cpp

bin/string_functions.o : src/string_functions.cpp
	$(CC) $(CFLAGS) -o $@ src/string_functions.cpp

bin/drmap : bin/command_line.o bin/diskfile.o bin/drmap.o bin/grid_float.o bin/memory.o bin/r_figure.o bin/string_functions.o
	$(CC) $(LINKFLAGS) bin/command_line.o bin/diskfile.o bin/drmap.o bin/grid_float.o bin/memory.o bin/r_figure.o bin/string_functions.o $(LIBRARIES) \
	-o bin/drmap
	
drmap : bin/drmap

# clean everything
clean :
	rm bin/*
	
FORCE:
