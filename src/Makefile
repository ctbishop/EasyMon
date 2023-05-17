# Compile Command for EasyMon Program.


CC = gcc

TARGET = easymon

SOURCES = easymon.c helpers.c

HEADERS = easymon.h

CFLAGS = `pkg-config --cflags gtk4`

LDFLAGS = `pkg-config --libs gtk4`

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

