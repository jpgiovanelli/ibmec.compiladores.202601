/*
 * HomeScript - Analisador Léxico (Lexer)
 * Implementação
 *
 * Lê um arquivo .iot e identifica todos os tokens da linguagem HomeScript.
 * Suporta multi-idioma (português e inglês).
 * Não utiliza bibliotecas externas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

/* ---------- Funções Auxiliares (internas) ---------- */

/* Avança para o próximo caractere do arquivo */
static void lexer_avancar(Lexer *lexer) {
    if (lexer->eof) return;

    lexer->caractere_atual = fgetc(lexer->arquivo);
    if (lexer->caractere_atual == EOF) {
        lexer->eof = 1;
        lexer->caractere_atual = '\0';
    } else if (lexer->caractere_atual == '\n') {
        lexer->linha++;
        lexer->coluna = 0;
    } else {
        lexer->coluna++;
    }
}

/* Olha o próximo caractere sem consumir */
static char lexer_espiar(Lexer *lexer) {
    if (lexer->eof) return '\0';
    char c = fgetc(lexer->arquivo);
    if (c == EOF) return '\0';
    ungetc(c, lexer->arquivo);
    return c;
}

/* Pula espaços em branco */
static void lexer_pular_espacos(Lexer *lexer) {
    while (!lexer->eof && (lexer->caractere_atual == ' '  ||
                            lexer->caractere_atual == '\t' ||
                            lexer->caractere_atual == '\n' ||
                            lexer->caractere_atual == '\r')) {
        lexer_avancar(lexer);
    }
}

/* Pula comentários de linha (// ...) */
static void lexer_pular_comentario(Lexer *lexer) {
    if (lexer->caractere_atual == '/' && lexer_espiar(lexer) == '/') {
        while (!lexer->eof && lexer->caractere_atual != '\n') {
            lexer_avancar(lexer);
        }
        if (!lexer->eof) {
            lexer_avancar(lexer); /* pula o \n */
        }
    }
}

/* Pula espaços e comentários */
static void lexer_pular_insignificantes(Lexer *lexer) {
    while (!lexer->eof) {
        lexer_pular_espacos(lexer);
        if (lexer->caractere_atual == '/' && lexer_espiar(lexer) == '/') {
            lexer_pular_comentario(lexer);
        } else {
            break;
        }
    }
}

/* Cria um token com tipo e valor */
static Token criar_token(TokenType tipo, const char *valor, int linha, int coluna) {
    Token token;
    token.tipo = tipo;
    token.linha = linha;
    token.coluna = coluna;
    strncpy(token.valor, valor, MAX_TOKEN_LEN - 1);
    token.valor[MAX_TOKEN_LEN - 1] = '\0';
    return token;
}

/* Verifica se um identificador é uma palavra reservada */
static TokenType verificar_palavra_reservada(const char *palavra) {
    /* Palavras em inglês */
    if (strcmp(palavra, "device") == 0)        return TOKEN_DEVICE;
    if (strcmp(palavra, "sensor") == 0)        return TOKEN_SENSOR;
    if (strcmp(palavra, "pin") == 0)           return TOKEN_PIN;
    if (strcmp(palavra, "turn") == 0)          return TOKEN_TURN;
    if (strcmp(palavra, "on") == 0)            return TOKEN_ON;
    if (strcmp(palavra, "off") == 0)           return TOKEN_OFF;
    if (strcmp(palavra, "wait") == 0)          return TOKEN_WAIT;
    if (strcmp(palavra, "if") == 0)            return TOKEN_IF;
    if (strcmp(palavra, "when") == 0)          return TOKEN_WHEN;
    if (strcmp(palavra, "detected") == 0)      return TOKEN_DETECTED;
    if (strcmp(palavra, "not_detected") == 0)  return TOKEN_NOT_DETECTED;

    /* Palavras em português (sinônimos) */
    if (strcmp(palavra, "dispositivo") == 0)   return TOKEN_DEVICE;
    if (strcmp(palavra, "pino") == 0)          return TOKEN_PIN;
    if (strcmp(palavra, "ligar") == 0)         return TOKEN_LIGAR;
    if (strcmp(palavra, "desligar") == 0)      return TOKEN_DESLIGAR;
    if (strcmp(palavra, "esperar") == 0)       return TOKEN_WAIT;
    if (strcmp(palavra, "se") == 0)            return TOKEN_IF;
    if (strcmp(palavra, "quando") == 0)        return TOKEN_WHEN;
    if (strcmp(palavra, "detectado") == 0)     return TOKEN_DETECTED;
    if (strcmp(palavra, "nao_detectado") == 0) return TOKEN_NOT_DETECTED;

    return TOKEN_IDENTIFIER;
}

/* ---------- Leitura de Tokens Específicos ---------- */

/* Lê um identificador ou palavra reservada */
static Token lexer_ler_identificador(Lexer *lexer) {
    char buffer[MAX_TOKEN_LEN];
    int i = 0;
    int linha_inicio = lexer->linha;
    int coluna_inicio = lexer->coluna;

    while (!lexer->eof && (isalnum(lexer->caractere_atual) || lexer->caractere_atual == '_')) {
        if (i < MAX_TOKEN_LEN - 1) {
            buffer[i++] = lexer->caractere_atual;
        }
        lexer_avancar(lexer);
    }
    buffer[i] = '\0';

    TokenType tipo = verificar_palavra_reservada(buffer);
    return criar_token(tipo, buffer, linha_inicio, coluna_inicio);
}

/* Lê um número inteiro */
static Token lexer_ler_numero(Lexer *lexer) {
    char buffer[MAX_TOKEN_LEN];
    int i = 0;
    int linha_inicio = lexer->linha;
    int coluna_inicio = lexer->coluna;

    while (!lexer->eof && isdigit(lexer->caractere_atual)) {
        if (i < MAX_TOKEN_LEN - 1) {
            buffer[i++] = lexer->caractere_atual;
        }
        lexer_avancar(lexer);
    }
    buffer[i] = '\0';

    return criar_token(TOKEN_NUMBER, buffer, linha_inicio, coluna_inicio);
}

/* Lê um pino analógico (A0, A1, etc.) */
static Token lexer_ler_pino_analogico(Lexer *lexer) {
    char buffer[MAX_TOKEN_LEN];
    int i = 0;
    int linha_inicio = lexer->linha;
    int coluna_inicio = lexer->coluna;

    /* Já sabemos que começa com 'A' e o próximo é dígito */
    buffer[i++] = lexer->caractere_atual; /* 'A' */
    lexer_avancar(lexer);

    while (!lexer->eof && isdigit(lexer->caractere_atual)) {
        if (i < MAX_TOKEN_LEN - 1) {
            buffer[i++] = lexer->caractere_atual;
        }
        lexer_avancar(lexer);
    }
    buffer[i] = '\0';

    return criar_token(TOKEN_ANALOG_PIN, buffer, linha_inicio, coluna_inicio);
}

/* Lê um operador */
static Token lexer_ler_operador(Lexer *lexer) {
    char buffer[3];
    int linha_inicio = lexer->linha;
    int coluna_inicio = lexer->coluna;
    char proximo;

    buffer[0] = lexer->caractere_atual;
    buffer[1] = '\0';
    buffer[2] = '\0';

    switch (lexer->caractere_atual) {
        case '=':
            proximo = lexer_espiar(lexer);
            if (proximo == '=') {
                lexer_avancar(lexer);
                buffer[1] = '=';
                lexer_avancar(lexer);
                return criar_token(TOKEN_OP_EQUAL, "==", linha_inicio, coluna_inicio);
            }
            /* '=' sozinho é erro nesta linguagem */
            lexer_avancar(lexer);
            return criar_token(TOKEN_ERROR, "=", linha_inicio, coluna_inicio);

        case '!':
            proximo = lexer_espiar(lexer);
            if (proximo == '=') {
                lexer_avancar(lexer);
                lexer_avancar(lexer);
                return criar_token(TOKEN_OP_NOT_EQUAL, "!=", linha_inicio, coluna_inicio);
            }
            lexer_avancar(lexer);
            return criar_token(TOKEN_ERROR, "!", linha_inicio, coluna_inicio);

        case '>':
            proximo = lexer_espiar(lexer);
            if (proximo == '=') {
                lexer_avancar(lexer);
                lexer_avancar(lexer);
                return criar_token(TOKEN_OP_GREATER_EQUAL, ">=", linha_inicio, coluna_inicio);
            }
            lexer_avancar(lexer);
            return criar_token(TOKEN_OP_GREATER, ">", linha_inicio, coluna_inicio);

        case '<':
            proximo = lexer_espiar(lexer);
            if (proximo == '=') {
                lexer_avancar(lexer);
                lexer_avancar(lexer);
                return criar_token(TOKEN_OP_LESS_EQUAL, "<=", linha_inicio, coluna_inicio);
            }
            lexer_avancar(lexer);
            return criar_token(TOKEN_OP_LESS, "<", linha_inicio, coluna_inicio);

        default:
            lexer_avancar(lexer);
            return criar_token(TOKEN_ERROR, buffer, linha_inicio, coluna_inicio);
    }
}

/* ---------- Funções Públicas ---------- */

Lexer* lexer_criar(const char *caminho_arquivo) {
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) {
        fprintf(stderr, "Erro: falha ao alocar memória para o lexer.\n");
        return NULL;
    }

    lexer->arquivo = fopen(caminho_arquivo, "r");
    if (!lexer->arquivo) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo '%s'.\n", caminho_arquivo);
        free(lexer);
        return NULL;
    }

    lexer->linha = 1;
    lexer->coluna = 0;
    lexer->eof = 0;

    /* Lê o primeiro caractere */
    lexer_avancar(lexer);

    return lexer;
}

void lexer_destruir(Lexer *lexer) {
    if (lexer) {
        if (lexer->arquivo) {
            fclose(lexer->arquivo);
        }
        free(lexer);
    }
}

Token lexer_proximo_token(Lexer *lexer) {
    lexer_pular_insignificantes(lexer);

    if (lexer->eof) {
        return criar_token(TOKEN_EOF, "EOF", lexer->linha, lexer->coluna);
    }

    char c = lexer->caractere_atual;

    /* Pino analógico: 'A' seguido de dígito */
    if (c == 'A' && isdigit(lexer_espiar(lexer))) {
        return lexer_ler_pino_analogico(lexer);
    }

    /* Identificador ou palavra reservada */
    if (isalpha(c) || c == '_') {
        return lexer_ler_identificador(lexer);
    }

    /* Número */
    if (isdigit(c)) {
        return lexer_ler_numero(lexer);
    }

    /* Operadores */
    if (c == '=' || c == '!' || c == '>' || c == '<') {
        return lexer_ler_operador(lexer);
    }

    /* Delimitadores */
    if (c == '{') {
        lexer_avancar(lexer);
        return criar_token(TOKEN_LBRACE, "{", lexer->linha, lexer->coluna);
    }
    if (c == '}') {
        lexer_avancar(lexer);
        return criar_token(TOKEN_RBRACE, "}", lexer->linha, lexer->coluna);
    }
    if (c == ';') {
        lexer_avancar(lexer);
        return criar_token(TOKEN_SEMICOLON, ";", lexer->linha, lexer->coluna);
    }

    /* Token inválido */
    char erro[2] = {c, '\0'};
    int linha = lexer->linha;
    int coluna = lexer->coluna;
    lexer_avancar(lexer);
    return criar_token(TOKEN_ERROR, erro, linha, coluna);
}

const char* token_tipo_nome(TokenType tipo) {
    switch (tipo) {
        case TOKEN_DEVICE:          return "KEYWORD_DEVICE";
        case TOKEN_SENSOR:          return "KEYWORD_SENSOR";
        case TOKEN_PIN:             return "KEYWORD_PIN";
        case TOKEN_TURN:            return "KEYWORD_TURN";
        case TOKEN_ON:              return "KEYWORD_ON";
        case TOKEN_OFF:             return "KEYWORD_OFF";
        case TOKEN_WAIT:            return "KEYWORD_WAIT";
        case TOKEN_IF:              return "KEYWORD_IF";
        case TOKEN_WHEN:            return "KEYWORD_WHEN";
        case TOKEN_DETECTED:        return "KEYWORD_DETECTED";
        case TOKEN_NOT_DETECTED:    return "KEYWORD_NOT_DETECTED";
        case TOKEN_LIGAR:           return "KEYWORD_LIGAR";
        case TOKEN_DESLIGAR:        return "KEYWORD_DESLIGAR";
        case TOKEN_IDENTIFIER:      return "IDENTIFIER";
        case TOKEN_NUMBER:          return "NUMBER";
        case TOKEN_ANALOG_PIN:      return "ANALOG_PIN";
        case TOKEN_OP_EQUAL:        return "OP_EQUAL";
        case TOKEN_OP_NOT_EQUAL:    return "OP_NOT_EQUAL";
        case TOKEN_OP_GREATER:      return "OP_GREATER";
        case TOKEN_OP_LESS:         return "OP_LESS";
        case TOKEN_OP_GREATER_EQUAL:return "OP_GREATER_EQUAL";
        case TOKEN_OP_LESS_EQUAL:   return "OP_LESS_EQUAL";
        case TOKEN_LBRACE:          return "DELIM_LBRACE";
        case TOKEN_RBRACE:          return "DELIM_RBRACE";
        case TOKEN_SEMICOLON:       return "DELIM_SEMICOLON";
        case TOKEN_EOF:             return "EOF";
        case TOKEN_ERROR:           return "ERRO";
        default:                    return "DESCONHECIDO";
    }
}

void token_imprimir(Token token) {
    printf("[Linha %3d, Col %3d] %-22s | %s\n",
           token.linha, token.coluna,
           token_tipo_nome(token.tipo),
           token.valor);
}
