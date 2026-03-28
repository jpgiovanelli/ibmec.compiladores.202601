/*
 * HomeScript - Árvore Sintática Abstrata (AST)
 * Arquivo de cabeçalho
 *
 * Define as estruturas que representam os nós da AST.
 */

#ifndef AST_H
#define AST_H

/* ---------- Tipos de Nó da AST ---------- */
typedef enum {
    NODE_PROGRAM,           /* Nó raiz: contém lista de declarações */
    NODE_DEVICE_DECL,       /* Declaração de dispositivo */
    NODE_SENSOR_DECL,       /* Declaração de sensor */
    NODE_TURN_CMD,          /* Comando turn on/off */
    NODE_WAIT_CMD,          /* Comando wait */
    NODE_IF_STMT,           /* Estrutura if */
    NODE_WHEN_STMT,         /* Estrutura when */
    NODE_BLOCK,             /* Bloco de comandos { } */
    NODE_CONDITION          /* Condição (sensor op valor) */
} NodeType;

/* ---------- Tipos de Operador ---------- */
typedef enum {
    OP_EQ,                  /* == */
    OP_NE,                  /* != */
    OP_GT,                  /* > */
    OP_LT,                 /* < */
    OP_GE,                  /* >= */
    OP_LE                   /* <= */
} OperatorType;

/* ---------- Tipos de Estado (on/off) ---------- */
typedef enum {
    STATE_ON,
    STATE_OFF
} DeviceState;

/* ---------- Nó genérico da AST ---------- */
#define MAX_NAME_LEN 64
#define MAX_CHILDREN 64

typedef struct ASTNode {
    NodeType tipo;

    /* Dados específicos do nó */
    char nome[MAX_NAME_LEN];           /* nome do dispositivo/sensor */
    char pino[MAX_NAME_LEN];           /* número ou nome do pino */
    OperatorType operador;             /* operador da condição */
    char valor_comparacao[MAX_NAME_LEN]; /* valor comparado */
    DeviceState estado;                /* on ou off */
    int tempo_espera;                  /* milissegundos para wait */

    /* Filhos */
    struct ASTNode *filhos[MAX_CHILDREN];
    int num_filhos;
} ASTNode;

/* ---------- Funções Públicas ---------- */

/* Cria um novo nó da AST */
ASTNode* ast_criar_no(NodeType tipo);

/* Adiciona um filho ao nó */
void ast_adicionar_filho(ASTNode *pai, ASTNode *filho);

/* Imprime a AST de forma indentada */
void ast_imprimir(ASTNode *no, int nivel);

/* Libera toda a memória da AST */
void ast_destruir(ASTNode *no);

/* Retorna o nome legível do tipo de nó */
const char* ast_tipo_nome(NodeType tipo);

/* Retorna o símbolo do operador */
const char* ast_operador_simbolo(OperatorType op);

#endif /* AST_H */
