#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../parser/ast.h"

#define SEMANTIC_MAX_ERRORS 128
#define SEMANTIC_MAX_ERROR_MSG 256

typedef struct {
    char mensagem[SEMANTIC_MAX_ERROR_MSG];
    int linha;
    int coluna;
} SemanticError;

typedef struct {
    int sucesso;
    SemanticError erros[SEMANTIC_MAX_ERRORS];
    int num_erros;
} SemanticResult;

void semantic_analisar(ASTNode *ast, SemanticResult *resultado);

#endif
