# HomeScript Compiler - Makefile
# Compila todos os módulos do compilador

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = homescript

# Diretórios
LEXER_DIR = ../lexer
PARSER_DIR = ../parser
CODEGEN_DIR = ../codegen
SEMANTIC_DIR = ../semantic

# Arquivos fonte
SRCS = main.c \
       $(LEXER_DIR)/lexer.c \
       $(PARSER_DIR)/parser.c \
       $(PARSER_DIR)/ast.c \
       $(SEMANTIC_DIR)/semantic.c \
       $(CODEGEN_DIR)/codegen.c

# Arquivo de saída
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Limpar
clean:
	rm -f $(TARGET) $(TARGET).exe

# Testar com exemplos
test: $(TARGET)
	@echo "=== Teste 1: teste.iot ==="
	./$(TARGET) ../exemplos/teste.iot
	@echo ""
	@echo "=== Teste 2: sensor_luz.iot ==="
	./$(TARGET) ../exemplos/sensor_luz.iot
	@echo ""
	@echo "=== Teste 3: automacao_completa.iot ==="
	./$(TARGET) ../exemplos/automacao_completa.iot

# Testar apenas tokens
test-tokens: $(TARGET)
	./$(TARGET) ../exemplos/teste.iot --tokens

# Testar apenas AST
test-ast: $(TARGET)
	./$(TARGET) ../exemplos/teste.iot --ast

# Testar saída JSON
test-json: $(TARGET)
	./$(TARGET) ../exemplos/teste.iot --json

# Gerar arquivo C
generate: $(TARGET)
	./$(TARGET) ../exemplos/teste.iot --output saida.c
	@echo "Arquivo saida.c gerado!"

.PHONY: all clean test test-tokens test-ast test-json generate
