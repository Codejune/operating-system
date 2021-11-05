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
CFLAGS = -c -W -Wall -Wextra -g $(INC)

# Execute program file
CFS = cfs
CFS_NICE = cfs_nice
REALTIME = realtime

# Source file
CFS_SRCS = cfs.c
CFS_NICE_SRCS = cfs_nice.c
REALTIME_SRCS = realtime.c
SRCS = $(CFS_SRCS CFS_NICE_SRCS REALTIME_SRCS)

# Object file
CFS_OBJS = $(CFS_SRCS:.c=.o)
CFS_NICE_OBJS = $(CFS_NICE_SRCS:.c=.o)
REALTIME_OBJS = $(REALTIME_SRCS:.c=.o)
OBJS = $(CFS_OBJS CFS_NICE_OBJS REALTIME_OBJS)

# Library file
LIBS = 

# Include path
INC =

# Execute file grneration
# $@ = TARGET
# $^ = DEPENDENCY
# make all: Make all execute file
all : $(OBJS)
	$(CC) -o $(CFS) $(CFS_OBJS) $(LIBS)
	$(CC) -o $(CFS_NICE) $(CFS_NICE_OBJS) $(LIBS)
	$(CC) -o $(REALTIME) $(REALTIME_OBJS) $(LIBS)
$(CFS) : $(CFS_OBJS)
	$(CC) -o $@ $^ $(LIBS)
$(CFS_NICE) : $(CFS_NICE_OBJS)
	$(CC) -o $@ $^ $(LIBS)
$(REALTIME) : $(REALTIME_OBJS)
	$(CC) -o $@ $^ $(LIBS)

# Object file generation
$(CFS_OBJS): 
	$(CC) $(CFLAGS) $(CFS_SRCS)
$(CFS_NICE_OBJS): 
	$(CC) $(CFLAGS) $(CFS_NICE_SRCS)
$(REALTIME_OBJS): 
	$(CC) $(CFLAGS) $(REALTIME_SRCS)

# make dep: Make dependency information file
dep:
	$(CC) -M $(INC) $(SRCS) > .dependency

# make new: Re-generation 
new:
	$(MAKE) clean
	$(MAKE) all

# make clean: Remove all generated file
clean:
	rm *.o $(SCHEDULE)
