/*
 * HomeScript - Gerador de Código (Code Generator)
 * Arquivo de cabeçalho
 *
 * Converte AST em código C/Arduino.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "../parser/ast.h"

/* Tamanho máximo do código gerado */
#define MAX_CODE_LEN 8192

/* ---------- Estrutura do Gerador ---------- */
typedef struct {
    char codigo[MAX_CODE_LEN];
    int posicao;
    int nivel_indentacao;
} CodeGenerator;

/* ---------- Funções Públicas ---------- */

/* Cria um gerador de código */
CodeGenerator* codegen_criar(void);

/* Libera recursos do gerador */
void codegen_destruir(CodeGenerator *gen);

/* Gera código C a partir da AST */
void codegen_gerar(CodeGenerator *gen, ASTNode *ast);

/* Retorna o código gerado como string */
const char* codegen_resultado(CodeGenerator *gen);

/* Salva o código gerado em um arquivo */
int codegen_salvar(CodeGenerator *gen, const char *caminho_arquivo);

#endif /* CODEGEN_H */
