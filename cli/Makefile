TARGET_DIR = ../build
TARGET = $(TARGET_DIR)/cli-reality-emu

DIRS = src

CFILES = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))

COBJFILES = $(CFILES:%.c=%.o)

CFLAGS = -Wall -O3 -Iinclude -I../include -fno-strict-aliasing

CC = gcc
AR = ar

all: $(TARGET) clean

$(TARGET): $(COBJFILES)
	$(CC) -o $(TARGET) $(COBJFILES) -L../build -lreality-emu -lSDL2 -lGL -lGLEW -lm -lpthread -O3
clean:
	rm $(COBJFILES)

$(COBJFILES): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
