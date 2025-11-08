# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -g

# Pastas
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Nome do executável final
TARGET = $(BIN_DIR)/tp3.out

# Lista de arquivos fonte e objetos
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Regra principal
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "✅ Executável gerado em $(TARGET)"

# Regra genérica para compilar .cpp -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "🔹 Compilado: $<"

# Limpeza
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "🧹 Limpeza completa"
