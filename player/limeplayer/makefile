CC = gcc
TARGET = limeplayer
SRC = limeplayer.c
LIBS = -llimesuiteng-legacyapi

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
