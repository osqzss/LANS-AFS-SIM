CC = gcc

SRCS = afs_sim.c afs_nav.c afs_rand.c \
./ldpc/alloc.c ./ldpc/mod2sparse.c ./rtklib/rtkcmn.c ./pocketsdr/pocketsdr.c \
./multipath/multipath.c
OBJS = $(SRCS:.c=.o)

INCLUDE = -I./ldpc -I./rtklib -I./pocketsdr -I./multipath
OPTIONS = -lm

WARNOPT = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function

CFLAGS = -Ofast $(INCLUDE) $(WARNOPT) $(OPTIONS) -g

TARGET = afs_sim

# Header files: changes to any of these trigger a rebuild of the affected objects.
HDRS_AFS_SIM = afs_nav.h afs_rand.h ./multipath/multipath.h
HDRS_AFS_NAV = afs_nav.h
HDRS_AFS_RAND = afs_rand.h
HDRS_MULTIPATH = ./multipath/multipath.h

all: $(TARGET)

# In GCC, the order of arguments matters, especially for linking libraries.
# The '-l' option must be placed *after* the source files or object files.
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(OPTIONS) -o $(TARGET)

# Per-file compilation rules with explicit header dependencies.
afs_sim.o: afs_sim.c $(HDRS_AFS_SIM)
	$(CC) $(CFLAGS) -c $< -o $@

afs_nav.o: afs_nav.c $(HDRS_AFS_NAV)
	$(CC) $(CFLAGS) -c $< -o $@

afs_rand.o: afs_rand.c $(HDRS_AFS_RAND)
	$(CC) $(CFLAGS) -c $< -o $@

./multipath/multipath.o: ./multipath/multipath.c $(HDRS_MULTIPATH)
	$(CC) $(CFLAGS) -c $< -o $@

# Remaining sources (ldpc, rtklib, pocketsdr) use the generic pattern rule.
./ldpc/alloc.o ./ldpc/mod2sparse.o ./rtklib/rtkcmn.o ./pocketsdr/pocketsdr.o:
./ldpc/%.o: ./ldpc/%.c
	$(CC) $(CFLAGS) -c $< -o $@

./rtklib/%.o: ./rtklib/%.c
	$(CC) $(CFLAGS) -c $< -o $@

./pocketsdr/%.o: ./pocketsdr/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.exe
