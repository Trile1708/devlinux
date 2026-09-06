CC = gcc
CFLAGS = -Wall -Wextra -pthread

TARGET = booking

all: $(TARGET)

$(TARGET): booking.c
	$(CC) $(CFLAGS) booking.c -o $(TARGET)

clean:
	rm -f $(TARGET)
