CC = gcc

SRCS = afs_sim.c afs_nav.c afs_rand.c ./ldpc/alloc.c ./ldpc/mod2sparse.c ./rtklib/rtkcmn.c ./pocketsdr/pocketsdr.c
OBJS = $(SRCS:.c=.o)

INCLUDE = -I./ldpc -I./rtklib -I./pocketsdr
OPTIONS = -fopenmp

WARNOPT = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function

CFLAGS = -Ofast $(INCLUDE) $(WARNOPT) $(OPTIONS) -g

TARGET = afs_sim

all: $(TARGET)

# Linker rule with -fopenmp option
$(TARGET):$(OBJS)
	$(CC) $(OPTIONS) $(OBJS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET).exe
