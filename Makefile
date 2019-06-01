TARGET_DIR = build
TARGET = $(TARGET_DIR)/reality-emu

DIRS = src src/r4300

CFILES = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))
COBJFILES = $(CFILES:%.c=%.o)
CFLAGS = $(foreach DIR,$(DIRS),-I$(DIR)) -Wall

CC = gcc

all: $(TARGET) clean

$(TARGET): $(COBJFILES)
	$(CC) -o $(TARGET) $(COBJFILES) -lSDL2 -lGL -lGLEW -lm -lpthread
clean:
	rm $(COBJFILES)

$(COBJFILES): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@