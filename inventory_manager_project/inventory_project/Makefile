# Makefile – Hybrid Inventory Manager
# C files compile as C11; C++ files compile as C++17.

CXX      := g++
CC       := gcc

CFLAGS   := -std=c11   -Wall -Wextra -pedantic -Iinclude
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Iinclude

SRC_DIR  := src
OBJ_DIR  := build
BIN      := inventory_manager

# Source files
C_SRCS   := $(SRC_DIR)/inventory.c
CXX_SRCS := $(SRC_DIR)/InventoryManager.cpp $(SRC_DIR)/main.cpp

# Object files
C_OBJS   := $(patsubst $(SRC_DIR)/%.c,   $(OBJ_DIR)/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CXX_SRCS))
ALL_OBJS := $(C_OBJS) $(CXX_OBJS)

.PHONY: all clean

all: $(OBJ_DIR) $(BIN)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile C sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link with g++ (handles C++ runtime automatically)
$(BIN): $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful  →  ./$(BIN)"

clean:
	rm -rf $(OBJ_DIR) $(BIN) inventory.dat
	@echo "Cleaned."
