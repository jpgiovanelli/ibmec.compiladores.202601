/*
 * HomeScript - Compilador Principal
 *
 * Pipeline completo: .iot → Tokens → AST → Código C
 *
 * Uso:
 *   homescript <arquivo.iot> [opções]
 *
 * Opções:
 *   --tokens    Mostrar apenas os tokens (análise léxica)
 *   --ast       Mostrar apenas a AST (análise sintática)
 *   --code      Mostrar apenas o código C gerado
 *   --output    Salvar código C em arquivo (padrão: saida.c)
 *   --json      Saída em formato JSON (para integração com backend)
 *   (sem opção) Mostrar tudo: tokens + AST + código C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"
#include "../codegen/codegen.h"
#include "../semantic/semantic.h"

/* ---------- Opções de Linha de Comando ---------- */
typedef struct {
    const char *arquivo_entrada;
    const char *arquivo_saida;
    int mostrar_tokens;
    int mostrar_ast;
    int mostrar_code;
    int formato_json;
    int mostrar_tudo;
} Opcoes;

static void imprimir_uso(void) {
    printf("HomeScript Compiler v1.0\n");
    printf("Uso: homescript <arquivo.iot> [opcoes]\n\n");
    printf("Opcoes:\n");
    printf("  --tokens    Mostrar apenas os tokens\n");
    printf("  --ast       Mostrar apenas a AST\n");
    printf("  --code      Mostrar apenas o codigo C gerado\n");
    printf("  --output F  Salvar codigo C no arquivo F\n");
    printf("  --json      Saida em formato JSON\n");
    printf("  --help      Mostrar esta ajuda\n");
}

static Opcoes parse_args(int argc, char *argv[]) {
    Opcoes opts;
    int i;
    memset(&opts, 0, sizeof(Opcoes));
    opts.mostrar_tudo = 1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            imprimir_uso();
            exit(0);
        } else if (strcmp(argv[i], "--tokens") == 0) {
            opts.mostrar_tokens = 1;
            opts.mostrar_tudo = 0;
        } else if (strcmp(argv[i], "--ast") == 0) {
            opts.mostrar_ast = 1;
            opts.mostrar_tudo = 0;
        } else if (strcmp(argv[i], "--code") == 0) {
            opts.mostrar_code = 1;
            opts.mostrar_tudo = 0;
        } else if (strcmp(argv[i], "--json") == 0) {
            opts.formato_json = 1;
        } else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                opts.arquivo_saida = argv[++i];
            } else {
                fprintf(stderr, "Erro: --output requer um nome de arquivo.\n");
                exit(1);
            }
        } else if (argv[i][0] != '-') {
            opts.arquivo_entrada = argv[i];
        } else {
            fprintf(stderr, "Opcao desconhecida: %s\n", argv[i]);
            imprimir_uso();
            exit(1);
        }
    }

    if (!opts.arquivo_entrada) {
        fprintf(stderr, "Erro: nenhum arquivo .iot especificado.\n\n");
        imprimir_uso();
        exit(1);
    }

    return opts;
}

/* ---------- Saída JSON ---------- */

/* Escapa string para JSON */
static void json_escape(const char *str, char *out, int max_len) {
    int i = 0, j = 0;
    while (str[i] && j < max_len - 2) {
        if (str[i] == '"' || str[i] == '\\') {
            out[j++] = '\\';
        }
        if (str[i] == '\n') {
            out[j++] = '\\';
            out[j++] = 'n';
            i++;
            continue;
        }
        out[j++] = str[i++];
    }
    out[j] = '\0';
}

/* Imprime tokens em JSON */
static void imprimir_tokens_json(const char *arquivo) {
    Lexer *lexer = lexer_criar(arquivo);
    if (!lexer) return;

    Token token;
    int primeiro = 1;

    printf("  \"tokens\": [\n");
    do {
        token = lexer_proximo_token(lexer);
        if (!primeiro) printf(",\n");
        primeiro = 0;

        printf("    {\"tipo\": \"%s\", \"valor\": \"%s\", \"linha\": %d, \"coluna\": %d}",
               token_tipo_nome(token.tipo), token.valor, token.linha, token.coluna);
    } while (token.tipo != TOKEN_EOF);
    printf("\n  ]");

    lexer_destruir(lexer);
}

/* Imprime AST em JSON (recursivo) */
static void ast_json(ASTNode *no, int nivel) {
    int i;
    char indent[128] = "";

    for (i = 0; i < nivel; i++) strcat(indent, "    ");

    printf("%s{\n", indent);
    printf("%s    \"tipo\": \"%s\"", indent, ast_tipo_nome(no->tipo));
    printf(",\n%s    \"linha\": %d", indent, no->linha);
    printf(",\n%s    \"coluna\": %d", indent, no->coluna);

    if (no->nome[0]) printf(",\n%s    \"nome\": \"%s\"", indent, no->nome);
    if (no->pino[0]) printf(",\n%s    \"pino\": \"%s\"", indent, no->pino);
    if (no->tipo == NODE_CONDITION) {
        printf(",\n%s    \"operador\": \"%s\"", indent, ast_operador_simbolo(no->operador));
        printf(",\n%s    \"valor\": \"%s\"", indent, no->valor_comparacao);
    }
    if (no->tipo == NODE_TURN_CMD) {
        printf(",\n%s    \"estado\": \"%s\"", indent, no->estado == STATE_ON ? "on" : "off");
    }
    if (no->tipo == NODE_WAIT_CMD) {
        printf(",\n%s    \"tempo\": %d", indent, no->tempo_espera);
    }
    if (no->tipo == NODE_VAR_DECL || no->tipo == NODE_ASSIGN_CMD || no->tipo == NODE_PRINT_CMD) {
        printf(",\n%s    \"expressao\": \"%s\"", indent, no->expressao);
    }

    if (no->num_filhos > 0) {
        printf(",\n%s    \"filhos\": [\n", indent);
        for (i = 0; i < no->num_filhos; i++) {
            if (i > 0) printf(",\n");
            ast_json(no->filhos[i], nivel + 2);
        }
        printf("\n%s    ]", indent);
    }

    printf("\n%s}", indent);
}

/* ---------- Fluxo de Execução ---------- */

/* Fase 1: Análise Léxica (tokens) */
static void fase_tokens(const char *arquivo, int json) {
    if (json) {
        imprimir_tokens_json(arquivo);
        return;
    }

    Lexer *lexer = lexer_criar(arquivo);
    if (!lexer) return;

    Token token;

    printf("=== ANALISE LEXICA (Tokens) ===\n\n");
    printf("%-8s %-8s %-22s   %s\n", "Linha", "Coluna", "Tipo", "Valor");
    printf("-------  ------  ----------------------   -----\n");

    do {
        token = lexer_proximo_token(lexer);
        token_imprimir(token);
    } while (token.tipo != TOKEN_EOF);

    printf("\n");

    lexer_destruir(lexer);
}

/* Fase 2: Análise Sintática (AST) */
static ASTNode* fase_parser(const char *arquivo, int *tem_erro, int *erro_linha, int *erro_coluna, char *mensagem_erro, size_t mensagem_len) {
    Lexer *lexer = lexer_criar(arquivo);
    if (!lexer) return NULL;

    Parser *parser = parser_criar(lexer);
    if (!parser) {
        lexer_destruir(lexer);
        return NULL;
    }

    ASTNode *ast = parser_analisar(parser);
    if (tem_erro) *tem_erro = parser_tem_erro(parser);
    if (erro_linha) *erro_linha = parser->erro_linha;
    if (erro_coluna) *erro_coluna = parser->erro_coluna;
    if (mensagem_erro && mensagem_len > 0) {
        strncpy(mensagem_erro, parser_erro_mensagem(parser), mensagem_len - 1);
        mensagem_erro[mensagem_len - 1] = '\0';
    }

    if (parser_tem_erro(parser)) {
        fprintf(stderr, "\nCompilacao interrompida devido a erros.\n");
    }

    parser_destruir(parser);
    lexer_destruir(lexer);

    return ast;
}

static void imprimir_erros_json(int syntax_error, int syntax_line, int syntax_col, const char *syntax_msg, const SemanticResult *sem_result) {
    int i;
    int primeiro = 1;
    char escaped[1024];

    printf("  \"erros\": [\n");

    if (syntax_error && syntax_msg && syntax_msg[0]) {
        json_escape(syntax_msg, escaped, sizeof(escaped));
        printf("    {\"fase\": \"sintatico\", \"mensagem\": \"%s\", \"linha\": %d, \"coluna\": %d}", escaped, syntax_line, syntax_col);
        primeiro = 0;
    }

    if (sem_result) {
        for (i = 0; i < sem_result->num_erros; i++) {
            if (!primeiro) printf(",\n");
            json_escape(sem_result->erros[i].mensagem, escaped, sizeof(escaped));
            printf("    {\"fase\": \"semantico\", \"mensagem\": \"%s\", \"linha\": %d, \"coluna\": %d}",
                   escaped, sem_result->erros[i].linha, sem_result->erros[i].coluna);
            primeiro = 0;
        }
    }

    printf("\n  ]");
}

/* Fase 3: Geração de Código */
static char* fase_codegen(ASTNode *ast) {
    CodeGenerator *gen = codegen_criar();
    if (!gen) return NULL;

    codegen_gerar(gen, ast);

    /* Copia o resultado para retornar */
    char *resultado = (char*)malloc(strlen(codegen_resultado(gen)) + 1);
    strcpy(resultado, codegen_resultado(gen));

    codegen_destruir(gen);
    return resultado;
}

/* ---------- Main ---------- */

int main(int argc, char *argv[]) {
    Opcoes opts = parse_args(argc, argv);
    ASTNode *ast;
    char *codigo_c;
    int syntax_error = 0;
    int syntax_line = 0;
    int syntax_col = 0;
    char syntax_msg[512] = "";
    int sem_error = 0;
    SemanticResult sem_result;
    int sucesso = 1;
    char erro_msg[1024] = "";
    char erro_escaped[2048];

    if (opts.formato_json) {
        /* Saída JSON completa */
        printf("{\n");

        /* Tokens */
        imprimir_tokens_json(opts.arquivo_entrada);
        printf(",\n");

        /* AST */
        ast = fase_parser(opts.arquivo_entrada, &syntax_error, &syntax_line, &syntax_col, syntax_msg, sizeof(syntax_msg));
        sucesso = !syntax_error;

        if (ast && !syntax_error) {
            semantic_analisar(ast, &sem_result);
            sem_error = !sem_result.sucesso;
            sucesso = sucesso && !sem_error;
        } else {
            memset(&sem_result, 0, sizeof(sem_result));
            sem_result.sucesso = 1;
        }

        if (syntax_error && syntax_msg[0]) {
            strncpy(erro_msg, syntax_msg, sizeof(erro_msg) - 1);
            erro_msg[sizeof(erro_msg) - 1] = '\0';
        } else if (sem_error && sem_result.num_erros > 0) {
            strncpy(erro_msg, sem_result.erros[0].mensagem, sizeof(erro_msg) - 1);
            erro_msg[sizeof(erro_msg) - 1] = '\0';
        }

        json_escape(erro_msg, erro_escaped, sizeof(erro_escaped));
        printf("  \"sucesso\": %s,\n", sucesso ? "true" : "false");
        printf("  \"erro\": \"%s\",\n", erro_escaped);
        imprimir_erros_json(syntax_error, syntax_line, syntax_col, syntax_msg, &sem_result);
        printf(",\n");

        if (ast) {
            printf("  \"ast\": ");
            ast_json(ast, 1);
            printf(",\n");

            /* Código C */
            if (sucesso) {
                codigo_c = fase_codegen(ast);
            } else {
                codigo_c = NULL;
            }

            if (codigo_c && sucesso) {
                char escaped[MAX_CODE_LEN * 2];
                json_escape(codigo_c, escaped, sizeof(escaped));
                printf("  \"codigo_c\": \"%s\"\n", escaped);
                free(codigo_c);
            } else {
                printf("  \"codigo_c\": \"\"\n");
            }

            ast_destruir(ast);
        } else {
            printf("  \"ast\": null,\n");
            printf("  \"codigo_c\": \"\"\n");
        }

        printf("}\n");
        return 0;
    }

    /* Saída normal */
    printf("============================================\n");
    printf("   HomeScript Compiler v1.0\n");
    printf("   Arquivo: %s\n", opts.arquivo_entrada);
    printf("============================================\n\n");

    /* Tokens */
    if (opts.mostrar_tudo || opts.mostrar_tokens) {
        fase_tokens(opts.arquivo_entrada, 0);
    }

    /* AST */
    ast = fase_parser(opts.arquivo_entrada, &syntax_error, &syntax_line, &syntax_col, syntax_msg, sizeof(syntax_msg));
    if (!ast) {
        fprintf(stderr, "Erro: falha ao gerar AST.\n");
        return 1;
    }
    if (syntax_error) {
        ast_destruir(ast);
        return 1;
    }

    semantic_analisar(ast, &sem_result);
    if (!sem_result.sucesso) {
        int i;
        fprintf(stderr, "Erros semanticos encontrados:\n");
        for (i = 0; i < sem_result.num_erros; i++) {
            fprintf(stderr, "  - linha %d, coluna %d: %s\n",
                    sem_result.erros[i].linha,
                    sem_result.erros[i].coluna,
                    sem_result.erros[i].mensagem);
        }
        ast_destruir(ast);
        return 1;
    }

    if (opts.mostrar_tudo || opts.mostrar_ast) {
        printf("=== ARVORE SINTATICA ABSTRATA (AST) ===\n\n");
        ast_imprimir(ast, 0);
        printf("\n");
    }

    /* Código C */
    codigo_c = fase_codegen(ast);
    if (codigo_c) {
        if (opts.mostrar_tudo || opts.mostrar_code) {
            printf("=== CODIGO C GERADO ===\n\n");
            printf("%s\n", codigo_c);
        }

        /* Salvar em arquivo */
        if (opts.arquivo_saida) {
            FILE *f = fopen(opts.arquivo_saida, "w");
            if (f) {
                fprintf(f, "%s", codigo_c);
                fclose(f);
                printf("Codigo C salvo em: %s\n", opts.arquivo_saida);
            } else {
                fprintf(stderr, "Erro: nao foi possivel salvar em '%s'.\n", opts.arquivo_saida);
            }
        }

        free(codigo_c);
    }

    ast_destruir(ast);

    printf("============================================\n");
    printf("   Compilacao concluida com sucesso!\n");
    printf("============================================\n");

    return 0;
}
