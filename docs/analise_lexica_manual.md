# Manual da Análise Léxica (HomeScript)

## Objetivo
Explicar, passo a passo, como o lexer do HomeScript transforma um arquivo `.iot` em sequência de tokens.

Arquivos de referência:
- `lexer/lexer.c`
- `lexer/lexer.h`

---

## 1. Entrada do lexer
1. O processo começa com `lexer_criar(caminho_arquivo)`.
2. O arquivo `.iot` é aberto em modo leitura.
3. O estado inicial é configurado:
   - `linha = 1`
   - `coluna = 0`
   - `eof = 0`
4. `lexer_avancar()` lê o primeiro caractere.

---

## 2. Estrutura principal de leitura
A cada chamada de `lexer_proximo_token(lexer)`, ocorre:
1. Remoção de espaços/comentários com `lexer_pular_insignificantes()`.
2. Se fim de arquivo, retorna `TOKEN_EOF`.
3. Classificação do caractere atual para escolher a função de leitura correta.

---

## 3. Filtro de insignificantes
`lexer_pular_insignificantes()` combina:
- `lexer_pular_espacos()` para ignorar ` `, `\t`, `\n`, `\r`
- `lexer_pular_comentario()` para ignorar comentários de linha iniciados por `//`

Resultado: o próximo token sempre começa em conteúdo relevante da linguagem.

---

## 4. Regras de reconhecimento de token
Ordem usada em `lexer_proximo_token`:

1. **Pino analógico** (`A0`, `A1`, ...)
   - condição: caractere atual `A` e próximo é dígito
   - função: `lexer_ler_pino_analogico()`
   - token: `TOKEN_ANALOG_PIN`

2. **Identificador/palavra reservada**
   - condição: letra ou `_`
   - função: `lexer_ler_identificador()`
   - regra interna: lê `[a-zA-Z_][a-zA-Z0-9_]*`
   - depois chama `verificar_palavra_reservada()`

3. **Número inteiro**
   - condição: dígito
   - função: `lexer_ler_numero()`
   - token: `TOKEN_NUMBER`

4. **Operador relacional/atribuição**
   - condição: `=`, `!`, `>`, `<`
   - função: `lexer_ler_operador()`
   - suportados: `==`, `=`, `!=`, `>`, `<`, `>=`, `<=`

5. **Operadores aritméticos e delimitadores simples**
   - `+`, `-`, `*`, `/`
   - `{`, `}`, `;`, `(`, `)`

6. **Fallback de erro**
   - qualquer caractere inválido gera `TOKEN_ERROR`

---

## 5. Palavras reservadas e sinônimos
A função `verificar_palavra_reservada()` mapeia lexemas para tipos de token.

### Inglês
`device`, `sensor`, `pin`, `let`, `print`, `turn`, `on`, `off`, `wait`, `if`, `when`, `detected`, `not_detected`

### Português
`dispositivo`, `pino`, `ligar`, `desligar`, `esperar`, `se`, `quando`, `detectado`, `nao_detectado`

Quando não é palavra reservada, retorna `TOKEN_IDENTIFIER`.

---

## 6. Controle de posição (linha e coluna)
A função `lexer_avancar()` também atualiza posição:
- se encontrar `\n`: incrementa linha e zera coluna
- caso contrário: incrementa coluna

Cada token guarda:
- `tipo`
- `valor`
- `linha`
- `coluna`

Isso permite mensagens de erro precisas no parser/semântico.

---

## 7. Exemplo prático (passo a passo)
Código de entrada:

```iot
device luz pin 13;
when movimento == detected {
    turn luz on;
}
```

Fluxo resumido:
1. `device` -> `TOKEN_DEVICE`
2. `luz` -> `TOKEN_IDENTIFIER`
3. `pin` -> `TOKEN_PIN`
4. `13` -> `TOKEN_NUMBER`
5. `;` -> `TOKEN_SEMICOLON`
6. `when` -> `TOKEN_WHEN`
7. `movimento` -> `TOKEN_IDENTIFIER`
8. `==` -> `TOKEN_OP_EQUAL`
9. `detected` -> `TOKEN_DETECTED`
10. `{` -> `TOKEN_LBRACE`
11. `turn` -> `TOKEN_TURN`
12. `luz` -> `TOKEN_IDENTIFIER`
13. `on` -> `TOKEN_ON`
14. `;` -> `TOKEN_SEMICOLON`
15. `}` -> `TOKEN_RBRACE`
16. fim de arquivo -> `TOKEN_EOF`

---

## 8. Estruturas de dados usadas
Em `lexer.h`:
- `enum TokenType`: catálogo de todos os tokens.
- `struct Token`: representa cada token individual.
- `struct Lexer`: estado global da leitura do arquivo.

---

## 9. Saída do lexer
A saída da análise léxica é uma lista ordenada de tokens consumida por:
1. Parser (`parser/`)
2. Análise semântica (`semantic/`)
3. Geração de código (`codegen/`)

Em resumo, o lexer é a camada que converte texto bruto em unidades sintáticas reconhecíveis pelo compilador.
