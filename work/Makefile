# Directory
MAKEFILE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Tools
CC = gcc

# Target
# FILENAME = testcalculator
FILENAME = testadd

all:
# Make toolchain
## Front End
	$(CC) -g ../fe/fe_main.c ../fe/decomposer.c -o fe

# Compile source file
	cp ../src/$(FILENAME).c ./$(FILENAME).c
	./fe ../src/$(FILENAME).c $(FILENAME).s
	chmod u+x fe

# Execution
	gcc $(FILENAME).s -o executable.elf

# Clean
clean:
	-find | grep -v Makefile | grep -v README.md | xargs rm -frv
