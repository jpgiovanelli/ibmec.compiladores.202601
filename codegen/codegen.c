/*
 * HomeScript - Gerador de Código (Code Generator)
 * Implementação
 *
 * Converte uma AST HomeScript em código C/Arduino.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

static void codegen_escrever(CodeGenerator *gen, const char *texto) {
    int len = strlen(texto);
    if (gen->posicao + len < MAX_CODE_LEN - 1) {
        strcpy(gen->codigo + gen->posicao, texto);
        gen->posicao += len;
    }
}

static void codegen_escrever_fmt(CodeGenerator *gen, const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    codegen_escrever(gen, buffer);
}

static void codegen_indentar(CodeGenerator *gen) {
    int i;
    for (i = 0; i < gen->nivel_indentacao; i++) {
        codegen_escrever(gen, "    ");
    }
}

static int eh_pino_analogico(const char *pino) {
    return pino[0] == 'A' && pino[1] >= '0' && pino[1] <= '9';
}

static int sensor_index(CodeGenerator *gen, const char *nome) {
    int i;
    for (i = 0; i < gen->num_sensores; i++) {
        if (strcmp(gen->sensores[i], nome) == 0) return i;
    }
    return -1;
}

static int usa_tipo_sensor(CodeGenerator *gen, const char *tipo) {
    int i;
    for (i = 0; i < gen->num_sensores; i++) {
        if (strcmp(gen->sensores_tipo[i], tipo) == 0) return 1;
    }
    return 0;
}

static void sensor_guardar(CodeGenerator *gen, ASTNode *no) {
    int idx;
    if (gen->num_sensores >= 64) return;

    idx = gen->num_sensores++;
    strncpy(gen->sensores[idx], no->nome, MAX_NAME_LEN - 1);
    gen->sensores[idx][MAX_NAME_LEN - 1] = '\0';

    strncpy(gen->sensores_tipo[idx], no->sensor_tipo, MAX_NAME_LEN - 1);
    gen->sensores_tipo[idx][MAX_NAME_LEN - 1] = '\0';

    strncpy(gen->sensores_pin1[idx], no->pino, MAX_NAME_LEN - 1);
    gen->sensores_pin1[idx][MAX_NAME_LEN - 1] = '\0';

    strncpy(gen->sensores_pin2[idx], no->pino_secundario, MAX_NAME_LEN - 1);
    gen->sensores_pin2[idx][MAX_NAME_LEN - 1] = '\0';
}

static void codegen_coletar_sensores(ASTNode *programa, CodeGenerator *gen) {
    int i;
    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_SENSOR_DECL) {
            sensor_guardar(gen, no);
        }
    }
}

static void codegen_funcoes_hcsr04(CodeGenerator *gen) {
    int i;
    for (i = 0; i < gen->num_sensores; i++) {
        if (strcmp(gen->sensores_tipo[i], "hcsr04") == 0) {
            codegen_escrever_fmt(gen, "long read_%s_cm() {\n", gen->sensores[i]);
            codegen_escrever_fmt(gen, "    digitalWrite(%s_trig_pin, LOW);\n", gen->sensores[i]);
            codegen_escrever(gen, "    delayMicroseconds(2);\n");
            codegen_escrever_fmt(gen, "    digitalWrite(%s_trig_pin, HIGH);\n", gen->sensores[i]);
            codegen_escrever(gen, "    delayMicroseconds(10);\n");
            codegen_escrever_fmt(gen, "    digitalWrite(%s_trig_pin, LOW);\n", gen->sensores[i]);
            codegen_escrever_fmt(gen, "    long duration = pulseIn(%s_echo_pin, HIGH, 30000);\n", gen->sensores[i]);
            codegen_escrever(gen, "    return duration / 58;\n");
            codegen_escrever(gen, "}\n\n");
        }
    }
}

static void codegen_declaracoes(CodeGenerator *gen, ASTNode *programa) {
    int i;
    int tem_device = 0;
    int tem_sensor = 0;

    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_DEVICE_DECL) {
            codegen_escrever_fmt(gen, "#define %s %s\n", no->nome, no->pino);
            tem_device = 1;
        }
    }

    if (tem_device) codegen_escrever(gen, "\n");

    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_SENSOR_DECL) {
            if (strcmp(no->sensor_tipo, "dht11") == 0) {
                codegen_escrever_fmt(gen, "DHT %s_dht(%s, DHT11);\n", no->nome, no->pino);
            } else if (strcmp(no->sensor_tipo, "hcsr04") == 0) {
                codegen_escrever_fmt(gen, "int %s_trig_pin = %s;\n", no->nome, no->pino);
                codegen_escrever_fmt(gen, "int %s_echo_pin = %s;\n", no->nome, no->pino_secundario);
            } else {
                codegen_escrever_fmt(gen, "int %s_pin = %s;\n", no->nome, no->pino);
            }
            tem_sensor = 1;
        }
        if (no->tipo == NODE_VAR_DECL) {
            codegen_escrever_fmt(gen, "int %s = %s;\n", no->nome, no->expressao);
        }
    }

    if (tem_sensor) codegen_escrever(gen, "\n");
}

static void codegen_setup(CodeGenerator *gen, ASTNode *programa) {
    int i;

    codegen_escrever(gen, "void setup() {\n");
    gen->nivel_indentacao = 1;

    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo == NODE_DEVICE_DECL) {
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "pinMode(%s, OUTPUT);\n", no->nome);
        }
    }

    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo != NODE_SENSOR_DECL) continue;

        if (strcmp(no->sensor_tipo, "dht11") == 0) {
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "%s_dht.begin();\n", no->nome);
        } else if (strcmp(no->sensor_tipo, "hcsr04") == 0) {
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "pinMode(%s_trig_pin, OUTPUT);\n", no->nome);
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "pinMode(%s_echo_pin, INPUT);\n", no->nome);
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "digitalWrite(%s_trig_pin, LOW);\n", no->nome);
        } else {
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "pinMode(%s_pin, INPUT);\n", no->nome);
        }
    }

    codegen_indentar(gen);
    codegen_escrever(gen, "Serial.begin(9600);\n");

    gen->nivel_indentacao = 0;
    codegen_escrever(gen, "}\n\n");
}

static void codegen_comando(CodeGenerator *gen, ASTNode *no) {
    int i;

    if (!no) return;

    switch (no->tipo) {
        case NODE_VAR_DECL:
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "int %s = %s;\n", no->nome, no->expressao);
            break;

        case NODE_ASSIGN_CMD:
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "%s = %s;\n", no->nome, no->expressao);
            break;

        case NODE_PRINT_CMD:
            codegen_indentar(gen);
            codegen_escrever_fmt(gen, "Serial.println(%s);\n", no->expressao);
            break;

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
            ASTNode *cond = no->filhos[0];
            ASTNode *bloco_then = no->filhos[1];
            ASTNode *bloco_else = (no->num_filhos >= 3) ? no->filhos[2] : NULL;
            int idx = sensor_index(gen, cond->nome);
            int detected = strcmp(cond->valor_comparacao, "detected") == 0 || strcmp(cond->valor_comparacao, "detectado") == 0;
            int not_detected = strcmp(cond->valor_comparacao, "not_detected") == 0 || strcmp(cond->valor_comparacao, "nao_detectado") == 0;

            codegen_indentar(gen);

            if (idx >= 0 && strcmp(gen->sensores_tipo[idx], "hcsr04") == 0) {
                if (detected) {
                    codegen_escrever_fmt(gen, "if (read_%s_cm() > 0) {\n", cond->nome);
                } else if (not_detected) {
                    codegen_escrever_fmt(gen, "if (read_%s_cm() == 0) {\n", cond->nome);
                } else {
                    codegen_escrever_fmt(gen, "if (read_%s_cm() %s %s) {\n",
                                        cond->nome,
                                        ast_operador_simbolo(cond->operador),
                                        cond->valor_comparacao);
                }
            } else if (idx >= 0 && strcmp(gen->sensores_tipo[idx], "dht11") == 0) {
                codegen_escrever_fmt(gen, "if (%s_dht.readTemperature() %s %s) {\n",
                                    cond->nome,
                                    ast_operador_simbolo(cond->operador),
                                    cond->valor_comparacao);
            } else if (detected) {
                codegen_escrever_fmt(gen, "if (digitalRead(%s_pin) == HIGH) {\n", cond->nome);
            } else if (not_detected) {
                codegen_escrever_fmt(gen, "if (digitalRead(%s_pin) == LOW) {\n", cond->nome);
            } else if (idx >= 0 && eh_pino_analogico(gen->sensores_pin1[idx])) {
                codegen_escrever_fmt(gen, "if (analogRead(%s_pin) %s %s) {\n",
                                    cond->nome,
                                    ast_operador_simbolo(cond->operador),
                                    cond->valor_comparacao);
            } else if (idx >= 0) {
                codegen_escrever_fmt(gen, "if (digitalRead(%s_pin) %s %s) {\n",
                                    cond->nome,
                                    ast_operador_simbolo(cond->operador),
                                    cond->valor_comparacao);
            } else {
                codegen_escrever_fmt(gen, "if (%s %s %s) {\n",
                                    cond->nome,
                                    ast_operador_simbolo(cond->operador),
                                    cond->valor_comparacao);
            }

            gen->nivel_indentacao++;
            for (i = 0; i < bloco_then->num_filhos; i++) {
                codegen_comando(gen, bloco_then->filhos[i]);
            }
            gen->nivel_indentacao--;

            codegen_indentar(gen);
            codegen_escrever(gen, "}\n");

            if (bloco_else) {
                codegen_indentar(gen);
                codegen_escrever(gen, "else {\n");
                gen->nivel_indentacao++;
                for (i = 0; i < bloco_else->num_filhos; i++) {
                    codegen_comando(gen, bloco_else->filhos[i]);
                }
                gen->nivel_indentacao--;
                codegen_indentar(gen);
                codegen_escrever(gen, "}\n");
            }
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

static void codegen_loop(CodeGenerator *gen, ASTNode *programa) {
    int i;

    codegen_escrever(gen, "void loop() {\n");
    gen->nivel_indentacao = 1;

    for (i = 0; i < programa->num_filhos; i++) {
        ASTNode *no = programa->filhos[i];
        if (no->tipo != NODE_DEVICE_DECL &&
            no->tipo != NODE_SENSOR_DECL &&
            no->tipo != NODE_VAR_DECL) {
            codegen_comando(gen, no);
        }
    }

    gen->nivel_indentacao = 0;
    codegen_escrever(gen, "}\n");
}

CodeGenerator* codegen_criar(void) {
    CodeGenerator *gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    if (!gen) {
        fprintf(stderr, "Erro: falha ao alocar memória para o gerador de código.\n");
        return NULL;
    }

    gen->codigo[0] = '\0';
    gen->posicao = 0;
    gen->nivel_indentacao = 0;
    gen->num_sensores = 0;

    return gen;
}

void codegen_destruir(CodeGenerator *gen) {
    if (gen) free(gen);
}

void codegen_gerar(CodeGenerator *gen, ASTNode *ast) {
    if (!gen || !ast) return;

    codegen_coletar_sensores(ast, gen);

    codegen_escrever(gen, "/*\n");
    codegen_escrever(gen, " * Código gerado automaticamente pelo compilador HomeScript\n");
    codegen_escrever(gen, " * Plataforma: Arduino/ESP32\n");
    codegen_escrever(gen, " */\n\n");
    codegen_escrever(gen, "#include <Arduino.h>\n");
    if (usa_tipo_sensor(gen, "dht11")) {
        codegen_escrever(gen, "#include <DHT.h>\n");
    }
    codegen_escrever(gen, "\n");

    codegen_declaracoes(gen, ast);

    if (usa_tipo_sensor(gen, "hcsr04")) {
        codegen_funcoes_hcsr04(gen);
    }

    codegen_setup(gen, ast);
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
