#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"

#define MAX_SYMBOLS 256
#define MAX_SCOPES 64

typedef enum {
    SYMBOL_DEVICE,
    SYMBOL_SENSOR,
    SYMBOL_VAR
} SymbolKind;

typedef struct {
    char nome[MAX_NAME_LEN];
    SymbolKind tipo;
    char pino[MAX_NAME_LEN];
    int linha;
    int coluna;
} Symbol;

typedef struct {
    int base;
} Scope;

typedef struct {
    SemanticResult *resultado;
    Symbol symbols[MAX_SYMBOLS];
    int num_symbols;
    Scope scopes[MAX_SCOPES];
    int scope_top;
} SemanticContext;

static void semantic_registrar_erro(SemanticContext *ctx, int linha, int coluna, const char *fmt, ...) {
    va_list args;

    if (!ctx || !ctx->resultado) return;
    if (ctx->resultado->num_erros >= SEMANTIC_MAX_ERRORS) return;

    SemanticError *erro = &ctx->resultado->erros[ctx->resultado->num_erros++];
    erro->linha = linha;
    erro->coluna = coluna;

    va_start(args, fmt);
    vsnprintf(erro->mensagem, sizeof(erro->mensagem), fmt, args);
    va_end(args);

    ctx->resultado->sucesso = 0;
}

static int is_number_str(const char *text) {
    int i;
    if (!text || !text[0]) return 0;
    for (i = 0; text[i]; i++) {
        if (!isdigit((unsigned char)text[i])) {
            return 0;
        }
    }
    return 1;
}

static int eh_palavra_condicional(const char *text) {
    return strcmp(text, "detected") == 0 ||
           strcmp(text, "detectado") == 0 ||
           strcmp(text, "not_detected") == 0 ||
           strcmp(text, "nao_detectado") == 0;
}

static int symbol_add(SemanticContext *ctx, const char *nome, SymbolKind tipo, const char *pino, int linha, int coluna) {
    Symbol *dest;

    if (ctx->num_symbols >= MAX_SYMBOLS) {
        semantic_registrar_erro(ctx, linha, coluna, "Limite de simbolos excedido (max %d)", MAX_SYMBOLS);
        return 0;
    }

    dest = &ctx->symbols[ctx->num_symbols++];
    strncpy(dest->nome, nome, MAX_NAME_LEN - 1);
    dest->nome[MAX_NAME_LEN - 1] = '\0';
    dest->tipo = tipo;
    dest->linha = linha;
    dest->coluna = coluna;

    if (pino) {
        strncpy(dest->pino, pino, MAX_NAME_LEN - 1);
        dest->pino[MAX_NAME_LEN - 1] = '\0';
    } else {
        dest->pino[0] = '\0';
    }

    return 1;
}

static Symbol* symbol_find_any(SemanticContext *ctx, const char *nome) {
    int i;
    for (i = 0; i < ctx->num_symbols; i++) {
        if (strcmp(ctx->symbols[i].nome, nome) == 0) {
            return &ctx->symbols[i];
        }
    }
    return NULL;
}

static Symbol* symbol_find_kind(SemanticContext *ctx, const char *nome, SymbolKind tipo) {
    int i;
    for (i = 0; i < ctx->num_symbols; i++) {
        if (ctx->symbols[i].tipo == tipo && strcmp(ctx->symbols[i].nome, nome) == 0) {
            return &ctx->symbols[i];
        }
    }
    return NULL;
}

static Symbol* symbol_find_var_visible(SemanticContext *ctx, const char *nome) {
    int i;
    for (i = ctx->num_symbols - 1; i >= 0; i--) {
        if (ctx->symbols[i].tipo == SYMBOL_VAR && strcmp(ctx->symbols[i].nome, nome) == 0) {
            return &ctx->symbols[i];
        }
    }
    return NULL;
}

static Symbol* symbol_find_var_in_current_scope(SemanticContext *ctx, const char *nome) {
    int i;
    int base = ctx->scopes[ctx->scope_top].base;

    for (i = ctx->num_symbols - 1; i >= base; i--) {
        if (ctx->symbols[i].tipo == SYMBOL_VAR && strcmp(ctx->symbols[i].nome, nome) == 0) {
            return &ctx->symbols[i];
        }
    }
    return NULL;
}

static void push_scope(SemanticContext *ctx) {
    if (ctx->scope_top + 1 >= MAX_SCOPES) {
        return;
    }
    ctx->scope_top++;
    ctx->scopes[ctx->scope_top].base = ctx->num_symbols;
}

static void pop_scope(SemanticContext *ctx) {
    int base;

    if (ctx->scope_top <= 0) return;
    base = ctx->scopes[ctx->scope_top].base;
    if (base >= 0 && base <= ctx->num_symbols) {
        ctx->num_symbols = base;
    }
    ctx->scope_top--;
}

static void validar_expressao(SemanticContext *ctx, ASTNode *no, const char *expr) {
    int i = 0;

    while (expr[i]) {
        if (isalpha((unsigned char)expr[i]) || expr[i] == '_') {
            char ident[MAX_NAME_LEN];
            int j = 0;

            while (expr[i] && (isalnum((unsigned char)expr[i]) || expr[i] == '_')) {
                if (j < MAX_NAME_LEN - 1) {
                    ident[j++] = expr[i];
                }
                i++;
            }
            ident[j] = '\0';

            if (!symbol_find_var_visible(ctx, ident)) {
                semantic_registrar_erro(
                    ctx,
                    no->linha,
                    no->coluna,
                    "Identificador '%s' usado na expressao sem declaracao de variavel",
                    ident
                );
            }
            continue;
        }
        i++;
    }
}

static void coletar_declaracoes_fixas(SemanticContext *ctx, ASTNode *no) {
    int i;
    Symbol *existente;

    if (!no) return;

    if (no->tipo == NODE_DEVICE_DECL || no->tipo == NODE_SENSOR_DECL) {
        SymbolKind tipo = (no->tipo == NODE_DEVICE_DECL) ? SYMBOL_DEVICE : SYMBOL_SENSOR;

        existente = symbol_find_any(ctx, no->nome);
        if (existente) {
            semantic_registrar_erro(
                ctx,
                no->linha,
                no->coluna,
                "Identificador '%s' ja declarado anteriormente na linha %d",
                no->nome,
                existente->linha
            );
        } else {
            for (i = 0; i < ctx->num_symbols; i++) {
                if ((ctx->symbols[i].tipo == SYMBOL_DEVICE || ctx->symbols[i].tipo == SYMBOL_SENSOR) &&
                    strcmp(ctx->symbols[i].pino, no->pino) == 0) {
                    semantic_registrar_erro(
                        ctx,
                        no->linha,
                        no->coluna,
                        "Pino '%s' ja esta em uso por '%s' (linha %d)",
                        no->pino,
                        ctx->symbols[i].nome,
                        ctx->symbols[i].linha
                    );
                    break;
                }
            }
            symbol_add(ctx, no->nome, tipo, no->pino, no->linha, no->coluna);
        }
    }

    for (i = 0; i < no->num_filhos; i++) {
        coletar_declaracoes_fixas(ctx, no->filhos[i]);
    }
}

static void analisar_no(SemanticContext *ctx, ASTNode *no) {
    int i;
    Symbol *s;

    if (!no) return;

    switch (no->tipo) {
        case NODE_PROGRAM:
            for (i = 0; i < no->num_filhos; i++) {
                analisar_no(ctx, no->filhos[i]);
            }
            break;

        case NODE_BLOCK:
            push_scope(ctx);
            for (i = 0; i < no->num_filhos; i++) {
                analisar_no(ctx, no->filhos[i]);
            }
            pop_scope(ctx);
            break;

        case NODE_VAR_DECL:
            if (symbol_find_any(ctx, no->nome) && !symbol_find_var_visible(ctx, no->nome)) {
                semantic_registrar_erro(
                    ctx,
                    no->linha,
                    no->coluna,
                    "Variavel '%s' conflita com um identificador nao-variavel ja declarado",
                    no->nome
                );
                break;
            }

            if (symbol_find_var_in_current_scope(ctx, no->nome)) {
                semantic_registrar_erro(
                    ctx,
                    no->linha,
                    no->coluna,
                    "Variavel '%s' ja declarada neste escopo",
                    no->nome
                );
                break;
            }

            validar_expressao(ctx, no, no->expressao);
            symbol_add(ctx, no->nome, SYMBOL_VAR, NULL, no->linha, no->coluna);
            break;

        case NODE_ASSIGN_CMD:
            if (!symbol_find_var_visible(ctx, no->nome)) {
                semantic_registrar_erro(
                    ctx,
                    no->linha,
                    no->coluna,
                    "Atribuicao para variavel '%s' nao declarada",
                    no->nome
                );
            }
            validar_expressao(ctx, no, no->expressao);
            break;

        case NODE_PRINT_CMD:
            validar_expressao(ctx, no, no->expressao);
            break;

        case NODE_TURN_CMD:
            s = symbol_find_kind(ctx, no->nome, SYMBOL_DEVICE);
            if (!s) {
                semantic_registrar_erro(
                    ctx,
                    no->linha,
                    no->coluna,
                    "Comando turn/ligar/desligar usa dispositivo '%s' nao declarado",
                    no->nome
                );
            }
            break;

        case NODE_WAIT_CMD:
            if (no->tempo_espera < 0) {
                semantic_registrar_erro(
                    ctx,
                    no->linha,
                    no->coluna,
                    "Tempo de espera invalido: %d",
                    no->tempo_espera
                );
            }
            break;

        case NODE_IF_STMT:
        case NODE_WHEN_STMT:
            if (no->num_filhos >= 1 && no->filhos[0] && no->filhos[0]->tipo == NODE_CONDITION) {
                ASTNode *cond = no->filhos[0];

                if (!symbol_find_kind(ctx, cond->nome, SYMBOL_SENSOR)) {
                    semantic_registrar_erro(
                        ctx,
                        cond->linha,
                        cond->coluna,
                        "Condicao usa sensor '%s' nao declarado",
                        cond->nome
                    );
                }

                if (!is_number_str(cond->valor_comparacao) &&
                    !eh_palavra_condicional(cond->valor_comparacao) &&
                    !symbol_find_var_visible(ctx, cond->valor_comparacao)) {
                    semantic_registrar_erro(
                        ctx,
                        cond->linha,
                        cond->coluna,
                        "Valor '%s' da condicao nao e numero, palavra reservada nem variavel declarada",
                        cond->valor_comparacao
                    );
                }
            }

            for (i = 1; i < no->num_filhos; i++) {
                analisar_no(ctx, no->filhos[i]);
            }
            break;

        case NODE_DEVICE_DECL:
        case NODE_SENSOR_DECL:
        case NODE_CONDITION:
            break;

        default:
            for (i = 0; i < no->num_filhos; i++) {
                analisar_no(ctx, no->filhos[i]);
            }
            break;
    }
}

void semantic_analisar(ASTNode *ast, SemanticResult *resultado) {
    SemanticContext ctx;

    if (!resultado) return;

    memset(resultado, 0, sizeof(*resultado));
    resultado->sucesso = 1;

    if (!ast) {
        resultado->sucesso = 0;
        if (resultado->num_erros < SEMANTIC_MAX_ERRORS) {
            resultado->erros[0].linha = 0;
            resultado->erros[0].coluna = 0;
            snprintf(resultado->erros[0].mensagem, sizeof(resultado->erros[0].mensagem),
                     "AST invalida para analise semantica");
            resultado->num_erros = 1;
        }
        return;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.resultado = resultado;
    ctx.scopes[0].base = 0;
    ctx.scope_top = 0;

    coletar_declaracoes_fixas(&ctx, ast);
    analisar_no(&ctx, ast);
}
