# build

CC = g++
CFLAGS = -g -Wall

default: carnivores

carnivores:

ce_allo.o: CE_Allosaurus.cpp CE_Allosaurus.h
		$(CC) $(CFLAGS) -c CE_Allosaurus.cpp
