#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


static void parser_avancar(Parser *parser) {
    parser->token_atual = lexer_proximo_token(parser->lexer);
}

static void ast_marcar_posicao(ASTNode *no, Parser *parser) {
    if (!no || !parser) return;
    no->linha = parser->token_atual.linha;
    no->coluna = parser->token_atual.coluna;
}

static void parser_registrar_erro(Parser *parser, const char *mensagem) {
    if (!parser->erro) {
        parser->erro_linha = parser->token_atual.linha;
        parser->erro_coluna = parser->token_atual.coluna;
        snprintf(parser->mensagem_erro, sizeof(parser->mensagem_erro),
                 "Erro na linha %d, coluna %d: %s (encontrado: '%s')",
                 parser->token_atual.linha, parser->token_atual.coluna,
                 mensagem, parser->token_atual.valor);
    }
    parser->erro = 1;
    fprintf(stderr, "%s\n", parser->mensagem_erro);
}

static int parser_verificar(Parser *parser, TokenType tipo) {
    return parser->token_atual.tipo == tipo;
}

static int parser_ident_igual(Parser *parser, const char *texto) {
    return parser->token_atual.tipo == TOKEN_IDENTIFIER &&
           strcmp(parser->token_atual.valor, texto) == 0;
}

static int parser_consumir(Parser *parser, TokenType tipo, const char *mensagem_erro) {
    if (parser->token_atual.tipo == tipo) {
        parser_avancar(parser);
        return 1;
    }
    parser_registrar_erro(parser, mensagem_erro);
    return 0;
}

static OperatorType token_para_operador(TokenType tipo) {
    switch (tipo) {
        case TOKEN_OP_EQUAL:         return OP_EQ;
        case TOKEN_OP_NOT_EQUAL:     return OP_NE;
        case TOKEN_OP_GREATER:       return OP_GT;
        case TOKEN_OP_LESS:          return OP_LT;
        case TOKEN_OP_GREATER_EQUAL: return OP_GE;
        case TOKEN_OP_LESS_EQUAL:    return OP_LE;
        default:                     return OP_EQ;
    }
}

static int eh_operador(TokenType tipo) {
    return tipo == TOKEN_OP_EQUAL ||
           tipo == TOKEN_OP_NOT_EQUAL ||
           tipo == TOKEN_OP_GREATER ||
           tipo == TOKEN_OP_LESS ||
           tipo == TOKEN_OP_GREATER_EQUAL ||
           tipo == TOKEN_OP_LESS_EQUAL;
}

static int parser_expressao(Parser *parser, char *saida, int max_saida);

static int expr_set(char *dest, int max_dest, const char *src) {
    if ((int)strlen(src) >= max_dest) return 0;
    strcpy(dest, src);
    return 1;
}

static int parser_fator(Parser *parser, char *saida, int max_saida) {
    char interno[MAX_EXPR_LEN];

    if (parser_verificar(parser, TOKEN_NUMBER) || parser_verificar(parser, TOKEN_IDENTIFIER)) {
        return expr_set(saida, max_saida, parser->token_atual.valor) ? (parser_avancar(parser), 1) : 0;
    }

    if (parser_verificar(parser, TOKEN_LPAREN)) {
        parser_avancar(parser); 
        if (!parser_expressao(parser, interno, sizeof(interno))) {
            return 0;
        }
        if (!parser_consumir(parser, TOKEN_RPAREN, "')' esperado para fechar expressão")) {
            return 0;
        }
        return snprintf(saida, max_saida, "(%s)", interno) < max_saida;
    }

    parser_registrar_erro(parser, "Fator inválido na expressão");
    return 0;
}

static int parser_termo(Parser *parser, char *saida, int max_saida) {
    char acumulado[MAX_EXPR_LEN];
    char temp[MAX_EXPR_LEN];
    char direito[MAX_EXPR_LEN];
    char operador[2];
    TokenType op;

    if (!parser_fator(parser, acumulado, sizeof(acumulado))) {
        return 0;
    }

    while (parser_verificar(parser, TOKEN_OP_MULT) || parser_verificar(parser, TOKEN_OP_DIV)) {
        op = parser->token_atual.tipo;
        operador[0] = (op == TOKEN_OP_MULT) ? '*' : '/';
        operador[1] = '\0';
        parser_avancar(parser);

        if (!parser_fator(parser, direito, sizeof(direito))) {
            return 0;
        }

        if (snprintf(temp, sizeof(temp), "%s %s %s", acumulado, operador, direito) >= (int)sizeof(temp)) {
            parser_registrar_erro(parser, "Expressão muito longa");
            return 0;
        }
        strcpy(acumulado, temp);
    }

    return expr_set(saida, max_saida, acumulado);
}

static int parser_expressao(Parser *parser, char *saida, int max_saida) {
    char acumulado[MAX_EXPR_LEN];
    char temp[MAX_EXPR_LEN];
    char direito[MAX_EXPR_LEN];
    char operador[2];
    TokenType op;

    if (!parser_termo(parser, acumulado, sizeof(acumulado))) {
        return 0;
    }

    while (parser_verificar(parser, TOKEN_OP_PLUS) || parser_verificar(parser, TOKEN_OP_MINUS)) {
        op = parser->token_atual.tipo;
        operador[0] = (op == TOKEN_OP_PLUS) ? '+' : '-';
        operador[1] = '\0';
        parser_avancar(parser);

        if (!parser_termo(parser, direito, sizeof(direito))) {
            return 0;
        }

        if (snprintf(temp, sizeof(temp), "%s %s %s", acumulado, operador, direito) >= (int)sizeof(temp)) {
            parser_registrar_erro(parser, "Expressão muito longa");
            return 0;
        }
        strcpy(acumulado, temp);
    }

    return expr_set(saida, max_saida, acumulado);
}


static ASTNode* parser_device_decl(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_DEVICE_DECL);
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
        parser_registrar_erro(parser, "Nome do dispositivo esperado");
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
    parser_avancar(parser);

    if (!parser_consumir(parser, TOKEN_PIN, "Palavra 'pin' esperada")) {
        ast_destruir(no);
        return NULL;
    }

    if (parser_verificar(parser, TOKEN_NUMBER)) {
        strncpy(no->pino, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);
    } else if (parser_verificar(parser, TOKEN_ANALOG_PIN)) {
        strncpy(no->pino, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);
    } else {
        parser_registrar_erro(parser, "Número do pino esperado");
        ast_destruir(no);
        return NULL;
    }

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após declaração de dispositivo")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_sensor_decl(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_SENSOR_DECL);
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
        parser_registrar_erro(parser, "Nome do sensor esperado");
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
    parser_avancar(parser);

    if (parser_verificar(parser, TOKEN_PIN)) {
        parser_avancar(parser);

        if (parser_verificar(parser, TOKEN_NUMBER) || parser_verificar(parser, TOKEN_ANALOG_PIN)) {
            strncpy(no->pino, parser->token_atual.valor, MAX_NAME_LEN - 1);
            parser_avancar(parser);
        } else {
            parser_registrar_erro(parser, "Número do pino esperado");
            ast_destruir(no);
            return NULL;
        }
    } else if (parser_ident_igual(parser, "type") || parser_ident_igual(parser, "tipo")) {
        parser_avancar(parser);

        if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
            parser_registrar_erro(parser, "Tipo de sensor esperado após 'type'");
            ast_destruir(no);
            return NULL;
        }
        strncpy(no->sensor_tipo, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);

        if (strcmp(no->sensor_tipo, "dht11") == 0) {
            if (!parser_consumir(parser, TOKEN_PIN, "Palavra 'pin' esperada para sensor dht11")) {
                ast_destruir(no);
                return NULL;
            }
            if (parser_verificar(parser, TOKEN_NUMBER)) {
                strncpy(no->pino, parser->token_atual.valor, MAX_NAME_LEN - 1);
                parser_avancar(parser);
            } else {
                parser_registrar_erro(parser, "Pino digital esperado para sensor dht11");
                ast_destruir(no);
                return NULL;
            }
        } else if (strcmp(no->sensor_tipo, "hcsr04") == 0) {
            if (!parser_ident_igual(parser, "trig") && !parser_ident_igual(parser, "gatilho")) {
                parser_registrar_erro(parser, "Palavra 'trig'/'gatilho' esperada para sensor hcsr04");
                ast_destruir(no);
                return NULL;
            }
            parser_avancar(parser);
            if (!parser_verificar(parser, TOKEN_NUMBER) && !parser_verificar(parser, TOKEN_ANALOG_PIN)) {
                parser_registrar_erro(parser, "Pino trig esperado para sensor hcsr04");
                ast_destruir(no);
                return NULL;
            }
            strncpy(no->pino, parser->token_atual.valor, MAX_NAME_LEN - 1);
            parser_avancar(parser);

            if (!parser_ident_igual(parser, "echo") && !parser_ident_igual(parser, "eco")) {
                parser_registrar_erro(parser, "Palavra 'echo'/'eco' esperada para sensor hcsr04");
                ast_destruir(no);
                return NULL;
            }
            parser_avancar(parser);
            if (!parser_verificar(parser, TOKEN_NUMBER) && !parser_verificar(parser, TOKEN_ANALOG_PIN)) {
                parser_registrar_erro(parser, "Pino echo esperado para sensor hcsr04");
                ast_destruir(no);
                return NULL;
            }
            strncpy(no->pino_secundario, parser->token_atual.valor, MAX_NAME_LEN - 1);
            parser_avancar(parser);
        } else {
            parser_registrar_erro(parser, "Tipo de sensor desconhecido (suportados: dht11, hcsr04)");
            ast_destruir(no);
            return NULL;
        }
    } else {
        parser_registrar_erro(parser, "Declaração de sensor inválida: esperado 'pin' ou 'type'");
        ast_destruir(no);
        return NULL;
    }

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após declaração de sensor")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_var_decl(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_VAR_DECL);
    char expr[MAX_EXPR_LEN];
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
        parser_registrar_erro(parser, "Nome da variável esperado após 'let'");
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
    parser_avancar(parser);

    if (!parser_consumir(parser, TOKEN_OP_ASSIGN, "'=' esperado na declaração de variável")) {
        ast_destruir(no);
        return NULL;
    }

    if (!parser_expressao(parser, expr, sizeof(expr))) {
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->expressao, expr, MAX_EXPR_LEN - 1);

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após declaração de variável")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_assign_cmd(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_ASSIGN_CMD);
    char expr[MAX_EXPR_LEN];
    ast_marcar_posicao(no, parser);

    if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
        parser_registrar_erro(parser, "Nome da variável esperado na atribuição");
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
    parser_avancar(parser);

    if (!parser_consumir(parser, TOKEN_OP_ASSIGN, "'=' esperado na atribuição")) {
        ast_destruir(no);
        return NULL;
    }

    if (!parser_expressao(parser, expr, sizeof(expr))) {
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->expressao, expr, MAX_EXPR_LEN - 1);

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após atribuição")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_assign_core(Parser *parser, int exigir_pontovirgula) {
    ASTNode *no = ast_criar_no(NODE_ASSIGN_CMD);
    char expr[MAX_EXPR_LEN];
    ast_marcar_posicao(no, parser);

    if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
        parser_registrar_erro(parser, "Nome da variável esperado na atribuição");
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
    parser_avancar(parser);

    if (!parser_consumir(parser, TOKEN_OP_ASSIGN, "'=' esperado na atribuição")) {
        ast_destruir(no);
        return NULL;
    }

    if (!parser_expressao(parser, expr, sizeof(expr))) {
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->expressao, expr, MAX_EXPR_LEN - 1);

    if (exigir_pontovirgula &&
        !parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após atribuição")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_print_cmd(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_PRINT_CMD);
    char expr[MAX_EXPR_LEN];
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    if (!parser_expressao(parser, expr, sizeof(expr))) {
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->expressao, expr, MAX_EXPR_LEN - 1);

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após print")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_turn_cmd(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_TURN_CMD);
    ast_marcar_posicao(no, parser);

    if (parser_verificar(parser, TOKEN_LIGAR)) {
        no->estado = STATE_ON;
        parser_avancar(parser);

        if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
            parser_registrar_erro(parser, "Nome do dispositivo esperado após 'ligar'");
            ast_destruir(no);
            return NULL;
        }
        strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);

    } else if (parser_verificar(parser, TOKEN_DESLIGAR)) {
        no->estado = STATE_OFF;
        parser_avancar(parser);

        if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
            parser_registrar_erro(parser, "Nome do dispositivo esperado após 'desligar'");
            ast_destruir(no);
            return NULL;
        }
        strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);

    } else {
        parser_avancar(parser); 

        if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
            parser_registrar_erro(parser, "Nome do dispositivo esperado após 'turn'");
            ast_destruir(no);
            return NULL;
        }
        strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);

        if (parser_verificar(parser, TOKEN_ON)) {
            no->estado = STATE_ON;
        } else if (parser_verificar(parser, TOKEN_OFF)) {
            no->estado = STATE_OFF;
        } else {
            parser_registrar_erro(parser, "'on' ou 'off' esperado");
            ast_destruir(no);
            return NULL;
        }
        parser_avancar(parser);
    }

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após comando turn")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_wait_cmd(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_WAIT_CMD);
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    if (!parser_verificar(parser, TOKEN_NUMBER)) {
        parser_registrar_erro(parser, "Número de milissegundos esperado após 'wait'");
        ast_destruir(no);
        return NULL;
    }
    no->tempo_espera = atoi(parser->token_atual.valor);
    parser_avancar(parser);

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após comando wait")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}


static ASTNode* parser_condicao(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_CONDITION);
    ast_marcar_posicao(no, parser);

    if (!parser_verificar(parser, TOKEN_IDENTIFIER)) {
        parser_registrar_erro(parser, "Nome do sensor esperado na condição");
        ast_destruir(no);
        return NULL;
    }
    strncpy(no->nome, parser->token_atual.valor, MAX_NAME_LEN - 1);
    parser_avancar(parser);

    if (!eh_operador(parser->token_atual.tipo)) {
        parser_registrar_erro(parser, "Operador esperado na condição (==, !=, >, <, >=, <=)");
        ast_destruir(no);
        return NULL;
    }
    no->operador = token_para_operador(parser->token_atual.tipo);
    parser_avancar(parser);


    if (parser_verificar(parser, TOKEN_NUMBER) ||
        parser_verificar(parser, TOKEN_IDENTIFIER) ||
        parser_verificar(parser, TOKEN_DETECTED) ||
        parser_verificar(parser, TOKEN_NOT_DETECTED)) {
        strncpy(no->valor_comparacao, parser->token_atual.valor, MAX_NAME_LEN - 1);
        parser_avancar(parser);
    } else {
        parser_registrar_erro(parser, "Valor esperado na condição");
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_statement(Parser *parser);

static ASTNode* parser_bloco(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_BLOCK);
    ast_marcar_posicao(no, parser);

    if (!parser_consumir(parser, TOKEN_LBRACE, "'{' esperado para abrir bloco")) {
        ast_destruir(no);
        return NULL;
    }

    while (!parser_verificar(parser, TOKEN_RBRACE) &&
           !parser_verificar(parser, TOKEN_EOF) &&
           !parser->erro) {
        ASTNode *stmt = parser_statement(parser);
        if (stmt) {
            ast_adicionar_filho(no, stmt);
        }
    }

    if (!parser_consumir(parser, TOKEN_RBRACE, "'}' esperado para fechar bloco")) {
        ast_destruir(no);
        return NULL;
    }

    return no;
}

static ASTNode* parser_if_stmt(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_IF_STMT);
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    ASTNode *cond = parser_condicao(parser);
    if (!cond) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, cond);

    ASTNode *bloco = parser_bloco(parser);
    if (!bloco) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, bloco);

    if (parser_verificar(parser, TOKEN_ELSE)) {
        parser_avancar(parser);
        bloco = parser_bloco(parser);
        if (!bloco) {
            ast_destruir(no);
            return NULL;
        }
        ast_adicionar_filho(no, bloco);
    }

    return no;
}

static ASTNode* parser_while_stmt(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_WHILE_STMT);
    ast_marcar_posicao(no, parser);

    parser_avancar(parser);

    ASTNode *cond = parser_condicao(parser);
    if (!cond) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, cond);

    ASTNode *bloco = parser_bloco(parser);
    if (!bloco) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, bloco);

    return no;
}

static ASTNode* parser_for_stmt(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_FOR_STMT);
    ASTNode *init;
    ASTNode *cond;
    ASTNode *update;
    ASTNode *bloco;
    ast_marcar_posicao(no, parser);

    parser_avancar(parser);

    init = parser_assign_core(parser, 1);
    if (!init) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, init);

    cond = parser_condicao(parser);
    if (!cond) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, cond);

    if (!parser_consumir(parser, TOKEN_SEMICOLON, "';' esperado após condição do for")) {
        ast_destruir(no);
        return NULL;
    }

    update = parser_assign_core(parser, 0);
    if (!update) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, update);

    bloco = parser_bloco(parser);
    if (!bloco) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, bloco);

    return no;
}

static ASTNode* parser_when_stmt(Parser *parser) {
    ASTNode *no = ast_criar_no(NODE_WHEN_STMT);
    ast_marcar_posicao(no, parser);

    parser_avancar(parser); 

    ASTNode *cond = parser_condicao(parser);
    if (!cond) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, cond);

    ASTNode *bloco = parser_bloco(parser);
    if (!bloco) {
        ast_destruir(no);
        return NULL;
    }
    ast_adicionar_filho(no, bloco);

    if (parser_verificar(parser, TOKEN_ELSE)) {
        parser_avancar(parser);
        bloco = parser_bloco(parser);
        if (!bloco) {
            ast_destruir(no);
            return NULL;
        }
        ast_adicionar_filho(no, bloco);
    }

    return no;
}

static ASTNode* parser_statement(Parser *parser) {
    if (parser->erro) return NULL;

    switch (parser->token_atual.tipo) {
        case TOKEN_DEVICE:
            return parser_device_decl(parser);
        case TOKEN_SENSOR:
            return parser_sensor_decl(parser);
        case TOKEN_LET:
            return parser_var_decl(parser);
        case TOKEN_PRINT:
            return parser_print_cmd(parser);
        case TOKEN_TURN:
            return parser_turn_cmd(parser);
        case TOKEN_LIGAR:
        case TOKEN_DESLIGAR:
            return parser_turn_cmd(parser);
        case TOKEN_IDENTIFIER:
            return parser_assign_cmd(parser);
        case TOKEN_WAIT:
            return parser_wait_cmd(parser);
        case TOKEN_IF:
            return parser_if_stmt(parser);
        case TOKEN_FOR:
            return parser_for_stmt(parser);
        case TOKEN_WHILE:
            return parser_while_stmt(parser);
        case TOKEN_WHEN:
            return parser_when_stmt(parser);
        case TOKEN_ELSE:
            parser_registrar_erro(parser, "Bloco 'else/senao' sem if/when correspondente");
            parser_avancar(parser);
            return NULL;
        case TOKEN_ERROR:
            parser_registrar_erro(parser, "Token inválido encontrado");
            parser_avancar(parser);
            return NULL;
        default:
            parser_registrar_erro(parser, "Comando não reconhecido");
            parser_avancar(parser); 
            return NULL;
    }
}



Parser* parser_criar(Lexer *lexer) {
    Parser *parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) {
        fprintf(stderr, "Erro: falha ao alocar memória para o parser.\n");
        return NULL;
    }

    parser->lexer = lexer;
    parser->erro = 0;
    parser->erro_linha = 0;
    parser->erro_coluna = 0;
    parser->mensagem_erro[0] = '\0';

    parser_avancar(parser);

    return parser;
}

void parser_destruir(Parser *parser) {
    if (parser) {
        free(parser);
    }
}

ASTNode* parser_analisar(Parser *parser) {
    ASTNode *programa = ast_criar_no(NODE_PROGRAM);
    if (programa) {
        programa->linha = 1;
        programa->coluna = 1;
    }

    while (!parser_verificar(parser, TOKEN_EOF) && !parser->erro) {
        ASTNode *stmt = parser_statement(parser);
        if (stmt) {
            ast_adicionar_filho(programa, stmt);
        }
    }

    return programa;
}

int parser_tem_erro(Parser *parser) {
    return parser->erro;
}

const char* parser_erro_mensagem(Parser *parser) {
    return parser->mensagem_erro;
}
