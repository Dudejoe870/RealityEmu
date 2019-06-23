TARGET_DIR = build
TARGET = $(TARGET_DIR)/reality-emu

DIRS = src src/r4300 src/rdp

CFILES = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))
COBJFILES = $(CFILES:%.c=%.o)
CFLAGS = -Wall -O3 -Iinclude

CC = gcc

all: $(TARGET) clean

$(TARGET): $(COBJFILES)
	$(CC) -o $(TARGET) $(COBJFILES) -lSDL2 -lGL -lGLEW -lm -lpthread -O3
clean:
	rm $(COBJFILES)

$(COBJFILES): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@