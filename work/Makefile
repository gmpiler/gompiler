# Directory
MAKEFILE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Tools
CC = gcc
C_OPT = -g

# Target
# FILENAME = testcalculator
FILENAME = testadd

all:
# Make toolchain
## Front End
	$(CC) -g -c ../fe/fe_main.c -o fe.o
	$(CC) -g -c ../fe/decomposer.c -o fe_decomposer.o

## Middle Path
	$(CC) -g -c ../mp/mp_main.c -o mp_main.o

## Back End
	$(CC) -g -c ../be/be_main.c -o be.o

## Link all
	$(CC) -g fe.o fe_decomposer.o mp_main.o be.o -o gompiler

# Compile source file
	cp ../src/$(FILENAME).c ./$(FILENAME).c
	./gompiler ../src/$(FILENAME).c $(FILENAME).s

# Clean
clean:
	-find | grep -v Makefile | grep -v README.md | xargs rm -frv
