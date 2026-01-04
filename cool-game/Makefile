#**************************************************************************************************
#
#   NEON VOID - Makefile
#
#   A survivor-like game built with raylib
#
#**************************************************************************************************

.PHONY: all clean run

# Define compiler and flags
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -O2

# raylib paths (relative to cool-game/)
RAYLIB_PATH = ..
RAYLIB_SRC = $(RAYLIB_PATH)/src
RAYLIB_LIB = $(RAYLIB_SRC)/libraylib.a

# Include paths
INCLUDE_PATHS = -I$(RAYLIB_SRC) -I$(RAYLIB_SRC)/external

# Platform-specific libraries (macOS)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
endif
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lGL -lm -lpthread -ldl -lrt -lX11
endif

# Source files
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

# Output
TARGET = neon_void

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(OBJS) $(RAYLIB_LIB) $(LDFLAGS) -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(RAYLIB_LIB):
	@echo "Building raylib library..."
	$(MAKE) -C $(RAYLIB_SRC)

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)
