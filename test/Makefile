# Directory
MAKEFILE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Tools
CC = gcc
C_OPT = -g

all:
# Make toolchain
## Front End
	$(CC) -g -c ../fe/fe_main.c -o fe.o
	$(CC) -g -c ../fe/decomposer.c -o fe_decomposer.o

## Middle Path
	$(CC) -g -c ../mp/mp_main.c -o mp_main.o
	$(CC) -g -c ../mp/stackmaker.c -o mp_stackmaker.o

## Back End
	$(CC) -g -c ../be/be_main.c -o be.o

## Link all
	$(CC) -g fe.o fe_decomposer.o mp_main.o mp_stackmaker.o be.o -o gompiler

# Clean
clean:
	-find | grep -v Makefile | grep -v README.md | grep -v testall.sh |  grep -v result.txt | xargs rm -frv
