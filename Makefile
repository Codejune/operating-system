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
LRU = lru
FIFO = fifo
OPT = opt
SC = sc

# Source file
COMMON_SRCS = common.c
LRU_SRCS = lru.c
FIFO_SRCS = fifo.c
OPT_SRCS = opt.c
SC_SRCS = sc.c
SRCS = $(COMMON_SRCS) $(LRU_SRCS) $(FIFO_SRCS) $(OPT_SRCS) $(SC_SRCS) 

# Object file
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
LRU_OBJS = $(LRU_SRCS:.c=.o)
FIFO_OBJS = $(FIFO_SRCS:.c=.o) 
OPT_OBJS = $(OPT_SRCS:.c=.o)
SC_OBJS = $(SC_SRCS:.c=.o)
OBJS = $(COMMON_OBJS) $(LRU_OBJS) $(FIFO_OBJS) $(OPT_OBJS) $(SC_OBJS)
# Library file
LIBS =
# Include path
INCS =

# Execute file grneration
# $@ = TARGET
# $^ = DEPENDENCY
all : $(OBJS)
	$(CC) -o $(LRU) $(LRU_OBJS) $(LIBS)
	$(CC) -o $(FIFO) $(COMMON_OBJS) $(FIFO_OBJS) $(LIBS)
	$(CC) -o $(OPT) $(OPT_OBJS) $(LIBS)
	$(CC) -o $(SC) $(SC_OBJS) $(LIBS)
$(LRU) : $(LRU_OBJS) 
	$(CC) -o $@ $^ $(LIBS)
$(FIFO) : $(FIFO_OBJS) $(COMMON_OBJS)
	$(CC) -o $@ $^ $(LIBS)
$(OPT) : $(OPT_OBJS) 
	$(CC) -o $@ $^ $(LIBS)
$(SC) : $(SC_OBJS) 
	$(CC) -o $@ $^ $(LIBS)

# Object file generation
# $(OBJS):
# 	$(CC) $(CFLAGS) $(SRCS)
# $(COMMON_OBJS): 
# 	$(CC) $(CFLAGS) $(COMMON_SRCS)
# $(LRU_OBJS): 
# 	$(CC) $(CFLAGS) $(LRU_SRCS)
# $(FIFO_OBJS): 
# 	$(CC) $(CFLAGS) $(FIFO_SRCS)
# $(OPT_OBJS): 
# 	$(CC) $(CFLAGS) $(OPT_SRCS)
# $(SC_OBJS): 
# 	$(CC) $(CFLAGS) $(SC_SRCS)

# make dep: Make dependency information file
dep:
	$(CC) -M $(INC) $(SRCS) > .dependency

# make new: Re-generation 
new:
	$(MAKE) clean
	$(MAKE) all

# make clean: Remove all generated file
clean:
	rm *.o $(LRU) $(FIFO) $(OPT) $(SC)
