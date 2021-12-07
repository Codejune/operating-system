.SUFFIXES: .c .o
.PHONY: dep all new clean

# Compiler
CC = gcc
# Compile option
# -c: Generate object file
# -W, -Wall: Print warning about all ambigous grammer 
# -Wextra: Print warning out of -W, -Wall
# -O2: Optimization
# -g: Debugging, PLEASE DELETE AFTER PROJECT COMPLETE!
CFLAGS = -c -W -Wall -Wextra -g $(INCS)

# Execute program file
INTERSECTION = intersection

# Source file
INTERSECTION_SRCS = intersection.c
SRCS = $(INTERSECTION_SRCS)

# Object file
INTERSECTION_OBJS = $(INTERSECTION_SRCS:.c=.o)
OBJS = $(INTERSECTION_OBJS)
# Library file
LIBS = -lpthread
# Include path
INCS =

# Execute file grneration
# $@ = TARGET
# $^ = DEPENDENCY
all : $(OBJS)
	$(CC) -o $(INTERSECTION) $(INTERSECTION_OBJS) $(LIBS)
$(INTERSECTION) : $(INTERSECTION_OBJS)
	$(CC) -o $@ $^ $(LIBS)

# make dep: Make dependency information file
dep:
	$(CC) -M $(INC) $(SRCS) > .dependency

# make new: Re-generation 
new:
	$(MAKE) clean
	$(MAKE) all

# make clean: Remove all generated file
clean:
	rm *.o $(INTERSECTION)