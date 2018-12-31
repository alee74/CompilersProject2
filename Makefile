# # # # # # # # # # # # # # # # # # # # # # #
#											#
#	Makefile								#
#											#
#	Target Executable:		reader			#
#											#
#	File Dependecies:		main.cpp		#
#							parser.h		#
#							parser.cpp		#
#							scanner.h		#
#							scanner.cpp		#
#											#
#	Creates Object Files:	main.o			#
#							parser.o		#
#							scanner.o		#
#											#
#	Written by:	Austin James Lee			#
#											#
# # # # # # # # # # # # # # # # # # # # # # #

OUT = alloc
CFLAGS = -Wall -pedantic -O2 -std=$(CPP)
CC = g++
CPP = c++11


$(OUT):			scanner.o parser.o allocator.o main.o
				$(CC) $(CFLAGS) -o $@ scanner.o parser.o allocator.o main.o

main.o:			main.cpp
				$(CC) $(CFLAGS) -c main.cpp

allocator.o:	allocator.h allocator.cpp
				$(CC) $(CFLAGS) -c allocator.cpp

parser.o:		parser.h parser.cpp
				$(CC) $(CFLAGS) -c parser.cpp

scanner.o:		scanner.h scanner.cpp
				$(CC) $(CFLAGS) -c scanner.cpp

.PHONY:			clean

clean:
				rm *.o
				rm $(OUT)

lines:
				wc -l *.h *.cpp | grep total
