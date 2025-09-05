# 简单的Makefile，用于直接构建项目

CC = cc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11
INCLUDES = -Iinclude
SRC = src/main.c src/c_x.c
TARGET = c_x

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

test: src/c_x.c tests/test_c_x.c
	$(CC) $(CFLAGS) $(INCLUDES) -o test_c_x $^
	./test_c_x

clean:
	rm -f $(TARGET) test_c_x

.PHONY: all test clean

