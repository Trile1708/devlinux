CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

TARGET	=	sensor_daemon
SRC	=	main.c

.PHONY:	all clean

all: $(TARGET)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
