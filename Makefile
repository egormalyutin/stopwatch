TARGET = stopwatch
PREFIX ?= /usr/bin
CC ?= gcc

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(TARGET)

$(TARGET): main.c
	$(CC) -o $(TARGET) main.c

install:
	install $(TARGET) $(PREFIX)

uninstall:
	rm -rf $(PREFIX)/$(TARGET)
