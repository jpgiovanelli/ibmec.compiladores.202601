/*
 * HomeScript - Árvore Sintática Abstrata (AST)
 * Implementação
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* ast_criar_no(NodeType tipo) {
    ASTNode *no = (ASTNode*)malloc(sizeof(ASTNode));
    if (!no) {
        fprintf(stderr, "Erro: falha ao alocar memória para nó AST.\n");
        return NULL;
    }

    no->tipo = tipo;
    no->nome[0] = '\0';
    no->pino[0] = '\0';
    no->operador = OP_EQ;
    no->valor_comparacao[0] = '\0';
    no->estado = STATE_ON;
    no->tempo_espera = 0;
    no->num_filhos = 0;

    int i;
    for (i = 0; i < MAX_CHILDREN; i++) {
        no->filhos[i] = NULL;
    }

    return no;
}

void ast_adicionar_filho(ASTNode *pai, ASTNode *filho) {
    if (!pai || !filho) return;

    if (pai->num_filhos < MAX_CHILDREN) {
        pai->filhos[pai->num_filhos++] = filho;
    } else {
        fprintf(stderr, "Erro: número máximo de filhos excedido.\n");
    }
}

const char* ast_tipo_nome(NodeType tipo) {
    switch (tipo) {
        case NODE_PROGRAM:      return "Program";
        case NODE_DEVICE_DECL:  return "DeviceDeclaration";
        case NODE_SENSOR_DECL:  return "SensorDeclaration";
        case NODE_TURN_CMD:     return "TurnCommand";
        case NODE_WAIT_CMD:     return "WaitCommand";
        case NODE_IF_STMT:      return "IfStatement";
        case NODE_WHEN_STMT:    return "WhenStatement";
        case NODE_BLOCK:        return "Block";
        case NODE_CONDITION:    return "Condition";
        default:                return "Unknown";
    }
}

const char* ast_operador_simbolo(OperatorType op) {
    switch (op) {
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        case OP_GT: return ">";
        case OP_LT: return "<";
        case OP_GE: return ">=";
        case OP_LE: return "<=";
        default:    return "??";
    }
}

/* Imprime espaços de indentação com caracteres de árvore */
static void imprimir_indentacao(int nivel, int ultimo) {
    int i;
    for (i = 0; i < nivel; i++) {
        if (i == nivel - 1) {
            printf(ultimo ? "  +-- " : "  |-- ");
        } else {
            printf("  |   ");
        }
    }
}

void ast_imprimir(ASTNode *no, int nivel) {
    if (!no) return;

    int i;

    if (nivel == 0) {
        printf("%s\n", ast_tipo_nome(no->tipo));
    } else {
        printf("%s", ast_tipo_nome(no->tipo));
    }

    /* Detalhes específicos do nó */
    switch (no->tipo) {
        case NODE_DEVICE_DECL:
            printf(" (nome: %s, pino: %s)\n", no->nome, no->pino);
            break;
        case NODE_SENSOR_DECL:
            printf(" (nome: %s, pino: %s)\n", no->nome, no->pino);
            break;
        case NODE_TURN_CMD:
            printf(" (dispositivo: %s, estado: %s)\n",
                   no->nome, no->estado == STATE_ON ? "ON" : "OFF");
            break;
        case NODE_WAIT_CMD:
            printf(" (tempo: %d ms)\n", no->tempo_espera);
            break;
        case NODE_CONDITION:
            printf(" (%s %s %s)\n",
                   no->nome, ast_operador_simbolo(no->operador), no->valor_comparacao);
            break;
        default:
            printf("\n");
            break;
    }

    /* Imprimir filhos */
    for (i = 0; i < no->num_filhos; i++) {
        int eh_ultimo = (i == no->num_filhos - 1);
        imprimir_indentacao(nivel + 1, eh_ultimo);
        ast_imprimir(no->filhos[i], nivel + 1);
    }
}

void ast_destruir(ASTNode *no) {
    if (!no) return;

    int i;
    for (i = 0; i < no->num_filhos; i++) {
        ast_destruir(no->filhos[i]);
    }

    free(no);
}
