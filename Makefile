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
MYTOP = mytop
MYPS = myps
MYLSCPU = mylscpu
# Source file
COMMON_SRCS = common.c
MYTOP_SRCS = mytop.c
MYPS_SRCS = myps.c
MYLSCPU_SRCS = mylscpu.c
SRCS = $(COMMON_SRCS) $(MYTOP_SRCS) $(MYPS_SRCS) $(MYLSCPU_SRCS) 
# Object file
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
MYTOP_OBJS = $(MYTOP_SRCS:.c=.o)
MYPS_OBJS = $(MYPS_SRCS:.c=.o)
MYLSCPU_OBJS = $(MYLSCPU_SRCS:.c=.o)
OBJS = $(MYTOP_OBJS) $(MYTOP_SRCS_OBJS) $(MYPS_OBJS)
# Library file
LIBS = -lncurses
# Include path
INC =

# Execute file grneration
# $@ = TARGET
# $^ = DEPENDENCY
# make all: Make all execute file
all : $(OBJS)
	$(CC) -o $(MYTOP) $(MYTOP_OBJS) $(LIBS)
	$(CC) -o $(MYPS) $(MYPS_OBJS) $(LIBS)
	$(CC) -o $(MYLSCPU) $(MYLSCPU_OBJS) $(LIBS)
$(MYTOP) : $(MYTOP_OBJS)
	$(CC) -o $@ $^ $(LIBS)
$(MYPS) : $(MYPS_OBJS)
	$(CC) -o $@ $^ $(LIBS)
$(MYLSCPU) : $(MYLSCPU_OBJS)
	$(CC) -o $@ $^ $(LIBS)

# Object file generation
$(MYTOP_OBJS): 
	$(CC) $(CFLAGS) $(MYTOP_SRCS)
$(MYPS_OBJS): 
	$(CC) $(CFLAGS) $(MYPS_SRCS)
$(MYLSCPU_OBJS): 
	$(CC) $(CFLAGS) $(MYLSCPU_SRCS)

# make dep: Make dependency information file
dep:
	$(CC) -M $(INC) $(SRCS) > .dependency

# make new: Re-generation 
new:
	$(MAKE) clean
	$(MAKE) all

# make clean: Remove all generated file
clean:
	rm *.o $(MYTOP) $(MYPS) $(MYLSCPU)
