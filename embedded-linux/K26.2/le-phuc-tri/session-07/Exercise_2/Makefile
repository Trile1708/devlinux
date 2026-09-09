CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

TARGET = iot_gateway
SRC = main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
