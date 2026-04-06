/*
 * HomeScript - Parser (Analisador Sintático)
 * Arquivo de cabeçalho
 *
 * Recebe tokens do lexer e constrói a AST.
 */

#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "ast.h"

/* ---------- Estrutura do Parser ---------- */
typedef struct {
    Lexer *lexer;
    Token token_atual;
    int erro;
    int erro_linha;
    int erro_coluna;
    char mensagem_erro[512];
} Parser;

/* ---------- Funções Públicas ---------- */

/* Cria um parser a partir de um lexer */
Parser* parser_criar(Lexer *lexer);

/* Libera recursos do parser */
void parser_destruir(Parser *parser);

/* Executa o parsing e retorna a AST raiz */
ASTNode* parser_analisar(Parser *parser);

/* Verifica se houve erros durante o parsing */
int parser_tem_erro(Parser *parser);

/* Retorna a mensagem de erro */
const char* parser_erro_mensagem(Parser *parser);

#endif /* PARSER_H */
