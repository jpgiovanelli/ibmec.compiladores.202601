/*
 * HomeScript - Analisador Léxico (Lexer)
 * Arquivo de cabeçalho
 * 
 * Define os tipos de tokens e a interface do lexer.
 */

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

/* ---------- Tipos de Token ---------- */
typedef enum {
    /* Palavras reservadas */
    TOKEN_DEVICE,           /* device / dispositivo */
    TOKEN_SENSOR,           /* sensor */
    TOKEN_PIN,              /* pin / pino */
    TOKEN_LET,              /* let */
    TOKEN_PRINT,            /* print */
    TOKEN_TURN,             /* turn */
    TOKEN_ON,               /* on */
    TOKEN_OFF,              /* off */
    TOKEN_WAIT,             /* wait / esperar */
    TOKEN_IF,               /* if / se */
    TOKEN_WHEN,             /* when / quando */
    TOKEN_DETECTED,         /* detected / detectado */
    TOKEN_NOT_DETECTED,     /* not_detected / nao_detectado */

    /* Multi-idioma (mapeados para os mesmos tokens acima) */
    TOKEN_LIGAR,            /* ligar (sinônimo de turn + on) */
    TOKEN_DESLIGAR,         /* desligar (sinônimo de turn + off) */

    /* Literais */
    TOKEN_IDENTIFIER,       /* nomes: luz, ventilador, etc */
    TOKEN_NUMBER,           /* números: 13, 1000, 30 */
    TOKEN_ANALOG_PIN,       /* pinos analógicos: A0, A1 */

    /* Operadores */
    TOKEN_OP_EQUAL,         /* == */
    TOKEN_OP_ASSIGN,        /* = */
    TOKEN_OP_NOT_EQUAL,     /* != */
    TOKEN_OP_GREATER,       /* > */
    TOKEN_OP_LESS,          /* < */
    TOKEN_OP_GREATER_EQUAL, /* >= */
    TOKEN_OP_LESS_EQUAL,    /* <= */
    TOKEN_OP_PLUS,          /* + */
    TOKEN_OP_MINUS,         /* - */
    TOKEN_OP_MULT,          /* * */
    TOKEN_OP_DIV,           /* / */

    /* Delimitadores */
    TOKEN_LBRACE,           /* { */
    TOKEN_RBRACE,           /* } */
    TOKEN_SEMICOLON,        /* ; */
    TOKEN_LPAREN,           /* ( */
    TOKEN_RPAREN,           /* ) */

    /* Especiais */
    TOKEN_EOF,              /* Fim do arquivo */
    TOKEN_ERROR             /* Token inválido */
} TokenType;

/* ---------- Estrutura do Token ---------- */
#define MAX_TOKEN_LEN 256

typedef struct {
    TokenType tipo;
    char valor[MAX_TOKEN_LEN];
    int linha;
    int coluna;
} Token;

/* ---------- Estrutura do Lexer ---------- */
typedef struct {
    FILE *arquivo;
    char caractere_atual;
    int linha;
    int coluna;
    int eof;
} Lexer;

/* ---------- Funções Públicas ---------- */

/* Inicializa o lexer a partir de um arquivo .iot */
Lexer* lexer_criar(const char *caminho_arquivo);

/* Libera recursos do lexer */
void lexer_destruir(Lexer *lexer);

/* Obtém o próximo token do arquivo */
Token lexer_proximo_token(Lexer *lexer);

/* Retorna o nome legível do tipo de token */
const char* token_tipo_nome(TokenType tipo);

/* Imprime um token formatado */
void token_imprimir(Token token);

#endif /* LEXER_H */
