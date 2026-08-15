CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = manager

all: $(TARGET)

$(TARGET): manager.c
	$(CC) $(CFLAGS) manager.c -o $(TARGET)

clean:
	rm -f $(TARGET)
