# 📋 Tabela de Tokens — HomeScript

## Palavras Reservadas

| Token          | Tipo               | Exemplo           |
|----------------|--------------------|--------------------|
| `device`       | KEYWORD_DEVICE     | `device luz pin 13;` |
| `sensor`       | KEYWORD_SENSOR     | `sensor temp pin A0;` |
| `pin`          | KEYWORD_PIN        | `device luz pin 13;` |
| `turn`         | KEYWORD_TURN       | `turn luz on;`     |
| `on`           | KEYWORD_ON         | `turn luz on;`     |
| `off`          | KEYWORD_OFF        | `turn luz off;`    |
| `wait`         | KEYWORD_WAIT       | `wait 1000;`       |
| `if`           | KEYWORD_IF         | `if temp > 30 { }` |
| `when`         | KEYWORD_WHEN       | `when mov == detected { }` |
| `detected`     | KEYWORD_DETECTED   | `when mov == detected` |
| `not_detected` | KEYWORD_NOT_DETECTED | `when mov == not_detected` |

## Palavras Reservadas — Multi-idioma (Sinônimos)

| Token          | Tipo               | Exemplo              |
|----------------|--------------------|-----------------------|
| `quando`       | KEYWORD_WHEN       | `quando mov == detected { }` |
| `se`           | KEYWORD_IF         | `se temp > 30 { }`  |
| `ligar`        | KEYWORD_TURN_ON    | `ligar luz;`         |
| `desligar`     | KEYWORD_TURN_OFF   | `desligar luz;`      |
| `esperar`      | KEYWORD_WAIT       | `esperar 1000;`      |
| `dispositivo`  | KEYWORD_DEVICE     | `dispositivo luz pino 13;` |
| `pino`         | KEYWORD_PIN        | `dispositivo luz pino 13;` |
| `detectado`    | KEYWORD_DETECTED   | `quando mov == detectado` |
| `nao_detectado`| KEYWORD_NOT_DETECTED | `quando mov == nao_detectado` |

## Identificadores

| Token          | Tipo               | Exemplo            |
|----------------|--------------------|--------------------|
| `<nome>`       | IDENTIFIER         | `luz`, `ventilador`, `movimento` |

## Literais Numéricos

| Token          | Tipo               | Exemplo            |
|----------------|--------------------|--------------------|
| `<inteiro>`    | NUMBER             | `13`, `1000`, `30` |

## Literais de Pino Analógico

| Token          | Tipo               | Exemplo            |
|----------------|--------------------|--------------------|
| `A<número>`    | ANALOG_PIN         | `A0`, `A1`, `A5`   |

## Operadores

| Token  | Tipo                   | Exemplo            |
|--------|------------------------|--------------------|
| `==`   | OP_EQUAL               | `temp == 30`       |
| `!=`   | OP_NOT_EQUAL           | `temp != 0`        |
| `>`    | OP_GREATER             | `temp > 30`        |
| `<`    | OP_LESS                | `temp < 25`        |
| `>=`   | OP_GREATER_EQUAL       | `temp >= 30`       |
| `<=`   | OP_LESS_EQUAL          | `temp <= 25`       |

## Delimitadores

| Token  | Tipo                   | Exemplo            |
|--------|------------------------|--------------------|
| `{`    | DELIM_LBRACE           | `when mov == detected {` |
| `}`    | DELIM_RBRACE           | `}`                |
| `;`    | DELIM_SEMICOLON        | `turn luz on;`     |

## Tokens Especiais

| Token  | Tipo                   | Descrição          |
|--------|------------------------|--------------------|
| `EOF`  | TOKEN_EOF              | Fim do arquivo     |
| `ERR`  | TOKEN_ERROR            | Token inválido     |
