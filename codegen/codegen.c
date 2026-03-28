/*
 * HomeScript - Gerador de Código (Code Generator)
 * Implementação
 *
 * Converte uma AST HomeScript em código C/Arduino.
 *
 * Mapeamento:
 *   device X pin N  → #define X N  +  pinMode(X, OUTPUT)
 *   sensor X pin N  → int X_pin = N  +  pinMode(X_pin, INPUT)
 *   turn X on       → digitalWrite(X, HIGH)
 *   turn X off      → digitalWrite(X, LOW)
 *   wait N          → delay(N)
 *   if cond { }     → if (cond) { }
 *   when cond { }   → if (cond) { }  (dentro do loop)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

/* ---------- Funções Auxiliares ---------- */

/* Escreve texto no buffer de código */
static void codegen_escrever(CodeGenerator *gen, const char *texto) {
    int len = strlen(texto);
    if (gen->posicao + len < MAX_CODE_LEN - 1) {
        strcpy(gen->codigo + gen->posicao, texto);
        gen->posicao += len;
    }
}

/* Escreve com formatação (similar a sprintf) */
static void codegen_escrever_fmt(CodeGenerator *gen, const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    codegen_escrever(gen, buffer);
}

/* Escreve indentação */
static void codegen_indentar(CodeGenerator *gen) {
    int i;
    for (i = 0; i < gen->nivel_indentacao; i++) {
        codegen_escrever(gen, "    ");
    }
}

/* Verifica se o pino é analógico */
static int eh_pino_analogico(const char *pino) {
    return pino[0] == 'A' && pino[1] >= '0' && pino[1] <= '9';
}

/* ---------- Geração por Tipo de Nó ---------- */

/* Coleta declarações de device e sensor */
static void codegen_declaracoes(CodeGenerator *gen, ASTNode *programa) {
    int i;
    int tem_device = 0;
    int tem_sensor = 0;

    /* Defines para dispositivos */
    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_DEVICE_DECL) {
            codegen_escrever_fmt(gen, "#define %s %s\n", no->nome, no->pino);
            tem_device = 1;
        }
    }

    if (tem_device) codegen_escrever(gen, "\n");

    /* Variáveis para sensores */
    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_SENSOR_DECL) {
            if (eh_pino_analogico(no->pino)) {
                codegen_escrever_fmt(gen, "int %s_pin = %s;\n", no->nome, no->pino);
            } else {
                codegen_escrever_fmt(gen, "int %s_pin = %s;\n", no->nome, no->pino);
            }
            tem_sensor = 1;
        }
    }

    if (tem_sensor) codegen_escrever(gen, "\n");
}

/* Gera a função setup() */
static void codegen_setup(CodeGenerator *gen, ASTNode *programa) {
    int i;

    codegen_escrever(gen, "void setup() {\n");
    gen->nivel_indentacao = 1;

    /* pinMode para dispositivos */
    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_DEVICE_DECL) {
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "pinMode(%s, OUTPUT);\n", no->nome);
        }
    }

    /* pinMode para sensores */
    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_SENSOR_DECL) {
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "pinMode(%s_pin, INPUT);\n", no->nome);
        }
    }

    codegen_indentar(gen);
    codegen_escrever(gen, "Serial.begin(9600);\n");

    gen->nivel_indentacao = 0;
    codegen_escrever(gen, "}\n\n");
}

/* Gera um comando de forma recursiva */
static void codegen_comando(CodeGenerator *gen, ASTNode *no) {
    int i;

    if (!no) return;

    switch (no->tipo) {
        case NODE_TURN_CMD:
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "digitalWrite(%s, %s);\n",
                                no->nome,
                                no->estado == STATE_ON ? "HIGH" : "LOW");
            break;

        case NODE_WAIT_CMD:
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "delay(%d);\n", no->tempo_espera);
            break;

        case NODE_IF_STMT:
        case NODE_WHEN_STMT: {
            ASTNode *cond = no->filhos[0]; /* condição */
            ASTNode *bloco = no->filhos[1]; /* bloco */

            codegen_indentar(gen);

            /* Leitura do sensor */
            if (strcmp(cond->valor_comparacao, "detected") == 0 ||
                strcmp(cond->valor_comparacao, "detectado") == 0) {
                codegen_escrever_fmt(gen, "if (digitalRead(%s_pin) == HIGH) {\n",
                                    cond->nome);
            } else if (strcmp(cond->valor_comparacao, "not_detected") == 0 ||
                       strcmp(cond->valor_comparacao, "nao_detectado") == 0) {
                codegen_escrever_fmt(gen, "if (digitalRead(%s_pin) == LOW) {\n",
                                    cond->nome);
            } else {
                codegen_escrever_fmt(gen, "if (analogRead(%s_pin) %s %s) {\n",
                                    cond->nome,
                                    ast_operador_simbolo(cond->operador),
                                    cond->valor_comparacao);
            }

            gen->nivel_indentacao++;
            for (i = 0; i < bloco->num_filhos; i++) {
                codegen_comando(gen, bloco->filhos[i]);
            }
            gen->nivel_indentacao--;

            codegen_indentar(gen);
            codegen_escrever(gen, "}\n");
            break;
        }

        case NODE_BLOCK:
            for (i = 0; i < no->num_filhos; i++) {
                codegen_comando(gen, no->filhos[i]);
            }
            break;

        default:
            break;
    }
}

/* Gera a função loop() */
static void codegen_loop(CodeGenerator *gen, ASTNode *programa) {
    int i;

    codegen_escrever(gen, "void loop() {\n");
    gen->nivel_indentacao = 1;

    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo != NODE_DEVICE_DECL && no->tipo != NODE_SENSOR_DECL) {
            codegen_comando(gen, no);
        }
    }

    gen->nivel_indentacao = 0;
    codegen_escrever(gen, "}\n");
}

/* ---------- Funções Públicas ---------- */

CodeGenerator* codegen_criar(void) {
    CodeGenerator *gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    if (!gen) {
        fprintf(stderr, "Erro: falha ao alocar memória para o gerador de código.\n");
        return NULL;
    }

    gen->codigo[0] = '\0';
    gen->posicao = 0;
    gen->nivel_indentacao = 0;

    return gen;
}

void codegen_destruir(CodeGenerator *gen) {
    if (gen) free(gen);
}

void codegen_gerar(CodeGenerator *gen, ASTNode *ast) {
    if (!gen || !ast) return;

    /* Cabeçalho */
    codegen_escrever(gen, "/*\n");
    codegen_escrever(gen, " * Código gerado automaticamente pelo compilador HomeScript\n");
    codegen_escrever(gen, " * Plataforma: Arduino/ESP32\n");
    codegen_escrever(gen, " */\n\n");
    codegen_escrever(gen, "#include <Arduino.h>\n\n");

    /* Declarações (defines e variáveis) */
    codegen_declaracoes(gen, ast);

    /* Função setup() */
    codegen_setup(gen, ast);

    /* Função loop() */
    codegen_loop(gen, ast);
}

const char* codegen_resultado(CodeGenerator *gen) {
    return gen->codigo;
}

int codegen_salvar(CodeGenerator *gen, const char *caminho_arquivo) {
    FILE *arquivo = fopen(caminho_arquivo, "w");
    if (!arquivo) {
        fprintf(stderr, "Erro: não foi possível criar o arquivo '%s'.\n", caminho_arquivo);
        return 0;
    }

    fprintf(arquivo, "%s", gen->codigo);
    fclose(arquivo);
    return 1;
}
