# Directory
MAKEFILE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Tools
CC = gcc

# Target
FILENAME = testhello

all:
# Make toolchain
## Front End
	$(CC) ../fe/fe_main.c ../fe/decomposer.c -o fe

# Compile source file
	./fe ../src/$(FILENAME).c $(FILENAME).ir

# Clean
clean:
	-find | grep -v Makefile | grep -v README.md | xargs rm -frv
