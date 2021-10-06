all: mycalculator
CC=gcc
mycalculator: mycalculator.c
	$(CC) -o $@ $< 
clean:
	rm mycalculator
