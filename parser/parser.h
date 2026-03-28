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

#define MAX_ERROS 20

/* ---------- Erro individual ---------- */
typedef struct {
    int linha;
    int coluna;
    char mensagem[256];
    char encontrado[64];   /* token que foi encontrado */
} ErroCompilacao;

/* ---------- Estrutura do Parser ---------- */
typedef struct {
    Lexer *lexer;
    Token token_atual;
    int erro;
    char mensagem_erro[512];       /* compatibilidade: 1º erro */

    /* Sistema de múltiplos erros */
    ErroCompilacao erros[MAX_ERROS];
    int num_erros;
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

/* Retorna a mensagem de erro (primeiro erro) */
const char* parser_erro_mensagem(Parser *parser);

/* Retorna o número total de erros */
int parser_num_erros(Parser *parser);

/* Retorna um erro pelo índice */
const ErroCompilacao* parser_obter_erro(Parser *parser, int indice);

#endif /* PARSER_H */
