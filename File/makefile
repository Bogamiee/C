# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = ls
SRCS = main.c myls.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c myls.h
	$(CC) $(CFLAGS) -c $< -o $@
	clear

clean:
	rm -f $(OBJS) $(TARGET)
	clear

.PHONY: all clean
