# Manual Completo da Fase Lexica - HomeScript

## 1) Objetivo deste documento
Este documento explica somente a **fase lexica** do compilador HomeScript.

Escopo:
- como o lexer le o arquivo `.iot`
- como ele reconhece tokens
- como ele trata espacos, comentarios e erros lexicos
- quais funcoes e estruturas participam dessa fase

Fora de escopo (ficara para outros documentos):
- analise sintatica (parser)
- analise semantica
- geracao de codigo

Arquivos-base usados:
- `lexer/lexer.h`
- `lexer/lexer.c`
- `docs/tabela_tokens.md`
- `docs/regex.md`

---

## 2) O que e a fase lexica (explicado de forma simples)
A fase lexica e a etapa que pega texto bruto e transforma em uma lista de pecas chamadas `tokens`.

Exemplo:

```iot
turn luz on;
```

Resultado lexicamente:
1. `turn` -> palavra-chave
2. `luz` -> identificador
3. `on` -> palavra-chave
4. `;` -> delimitador

---

## 3) Estruturas da fase lexica

### 3.1 `TokenType`
Lista com todos os tipos de token reconhecidos.

### 3.2 `Token`
Cada token tem:
- `tipo`
- `valor`
- `linha`
- `coluna`

### 3.3 `Lexer`
Estado interno do leitor:
- arquivo aberto
- caractere atual
- linha/coluna atual
- indicador de fim (`eof`)

---

## 4) Tokens reconhecidos pelo lexer

### 4.1 Palavras reservadas (ingles)
- `device`, `sensor`, `pin`, `let`, `print`, `turn`, `on`, `off`, `wait`, `if`, `when`, `detected`, `not_detected`

### 4.2 Palavras reservadas (portugues/sinonimos)
- `dispositivo`, `pino`, `ligar`, `desligar`, `esperar`, `se`, `quando`, `detectado`, `nao_detectado`

### 4.3 Literais
- `TOKEN_IDENTIFIER` (nomes criados pelo usuario)
- `TOKEN_NUMBER` (numeros inteiros)
- `TOKEN_ANALOG_PIN` (`A0`, `A1`, ...)

### 4.4 Operadores
- relacionais/atribuicao: `==`, `=`, `!=`, `>`, `<`, `>=`, `<=`
- aritmeticos: `+`, `-`, `*`, `/`

### 4.5 Delimitadores
- `{`, `}`, `;`, `(`, `)`

### 4.6 Especiais
- `TOKEN_EOF`
- `TOKEN_ERROR`

---

## 5) Regras lexicas formais (equivalentes a regex)

1. Identificador:
```regex
[a-zA-Z_][a-zA-Z0-9_]*
```

2. Numero inteiro:
```regex
[0-9]+
```

3. Pino analogico:
```regex
A[0-9]+
```

4. Operadores:
```regex
==|=|!=|>=|<=|>|<
```

5. Delimitadores:
```regex
[{};()]
```

6. Espacos ignorados:
```regex
[ \t\r\n]+
```

7. Comentario de linha:
```regex
//[^\n]*
```

---

## 6) Fluxo completo da fase lexica

### 6.1 Inicializacao
`lexer_criar(caminho)`:
1. aloca lexer
2. abre arquivo `.iot`
3. inicia `linha=1`, `coluna=0`, `eof=0`
4. le primeiro caractere com `lexer_avancar`

### 6.2 Avanco de leitura
`lexer_avancar`:
- le o proximo caractere
- atualiza linha/coluna
- marca fim de arquivo quando necessario

### 6.3 Espiar sem consumir
`lexer_espiar`:
- olha o proximo caractere sem avancar de fato
- usado para `==`, `!=`, `>=`, `<=`

### 6.4 Limpeza antes de reconhecer token
`lexer_pular_insignificantes`:
- chama `lexer_pular_espacos`
- chama `lexer_pular_comentario` quando encontra `//`
- repete ate achar caractere relevante

### 6.5 Reconhecimento principal
`lexer_proximo_token` segue esta ordem:
1. pula insignificantes
2. se fim, retorna `EOF`
3. `A` + digito -> `ANALOG_PIN`
4. letra ou `_` -> identificador/palavra reservada
5. digito -> numero
6. `= ! > <` -> operador relacional/atribuicao
7. `+ - * /` -> operador aritmetico
8. `{ } ; ( )` -> delimitador
9. qualquer outro caractere -> `TOKEN_ERROR`

---

## 7) Funcoes da fase lexica (detalhadas)

### `lexer_ler_identificador`
- le sequencia alfanumerica com `_`
- depois verifica se e palavra reservada
- retorna keyword ou `TOKEN_IDENTIFIER`

### `verificar_palavra_reservada`
- compara a palavra lida com lista fixa de keywords EN/PT
- se nao achar, retorna identificador

### `lexer_ler_numero`
- le apenas inteiros (digitos continuos)

### `lexer_ler_pino_analogico`
- le `A` seguido de um ou mais digitos

### `lexer_ler_operador`
- reconhece `==`, `=`, `!=`, `>`, `<`, `>=`, `<=`
- `!` sozinho vira erro lexico

### `criar_token`
- cria e preenche `Token`
- protege tamanho do texto com `MAX_TOKEN_LEN`

### `token_tipo_nome`
- converte tipo interno para string legivel

### `token_imprimir`
- imprime token em formato tabular

---

## 8) Exemplo completo da tokenizacao
Entrada:

```iot
// exemplo lexico
device luz pin 13;
when movimento == detected {
    turn luz on;
    wait 1000;
}
```

Saida esperada da fase lexica (ordem):
1. `device` -> `KEYWORD_DEVICE`
2. `luz` -> `IDENTIFIER`
3. `pin` -> `KEYWORD_PIN`
4. `13` -> `NUMBER`
5. `;` -> `DELIM_SEMICOLON`
6. `when` -> `KEYWORD_WHEN`
7. `movimento` -> `IDENTIFIER`
8. `==` -> `OP_EQUAL`
9. `detected` -> `KEYWORD_DETECTED`
10. `{` -> `DELIM_LBRACE`
11. `turn` -> `KEYWORD_TURN`
12. `luz` -> `IDENTIFIER`
13. `on` -> `KEYWORD_ON`
14. `;` -> `DELIM_SEMICOLON`
15. `wait` -> `KEYWORD_WAIT`
16. `1000` -> `NUMBER`
17. `;` -> `DELIM_SEMICOLON`
18. `}` -> `DELIM_RBRACE`
19. `EOF`

---

## 9) Erros lexicos
Quando aparece caractere nao reconhecido:
1. lexer cria `TOKEN_ERROR`
2. guarda linha/coluna
3. avanca para continuar leitura

Exemplos comuns:
- `@`
- `#`
- `!` sem `=`

---

## 10) Limites atuais da fase lexica
1. Nao reconhece numero decimal.
2. Nao reconhece strings com aspas.
3. So comentario de linha (`//`).
4. Pino analogico exige `A` + digito.
5. Lexema limitado a `MAX_TOKEN_LEN` (256).

---

## 11) Checklist final da fase lexica
1. Le arquivo caractere por caractere.
2. Ignora espacos e comentarios.
3. Identifica tokens conforme regras da linguagem.
4. Registra posicao (linha/coluna) de cada token.
5. Retorna `TOKEN_ERROR` para simbolos invalidos.
6. Finaliza com `TOKEN_EOF`.

Este checklist resume exatamente o comportamento da fase lexica implementada no projeto.
