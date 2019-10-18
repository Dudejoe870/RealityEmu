TARGET_DIR = build
TARGET = $(TARGET_DIR)/libreality-emu.a

DIRS = src src/mips src/r4300 src/rdp src/rsp

CFILES = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))

COBJFILES = $(CFILES:%.c=%.o)

CFLAGS = -Wall -O3 -Iinclude -fno-strict-aliasing -fPIC -lm -lpthread
CFLAGS_DBG = -Wall -g -Iinclude -fno-strict-aliasing -fPIC -lm -lpthread

CC = gcc
AR = ar
MAKE = make

all: $(TARGET) clean cli gui

$(TARGET): $(COBJFILES)
	$(AR) rcs $(TARGET) $(COBJFILES)
clean:
	rm $(COBJFILES)
cli: FORCE
	$(MAKE) -C cli all

$(COBJFILES): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

FORCE:
