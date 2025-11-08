# ================================
# Makefile for TP_2 project
# ================================

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -g

# Folders
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Executable name
TARGET = $(BIN_DIR)/tp3.out

# Source files and corresponding object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Default target
all: $(TARGET)

# Link objects into final executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJ_FILES) -o $(TARGET)
	@echo "✅ Build completo: $(TARGET)"

# Compile each .cpp into .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "🔹 Compilado: $<"

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "🧹 Limpeza completa."

# Run the program
run: all
	@echo "🚀 Executando programa..."
	@./$(TARGET)

.PHONY: all clean run
