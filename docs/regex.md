# 🔤 Expressões Regulares — HomeScript

## Identificadores
```
[a-zA-Z_][a-zA-Z0-9_]*
```

## Números Inteiros
```
[0-9]+
```

## Pinos Analógicos
```
A[0-9]+
```

## Operadores

| Operador | Regex       |
|----------|-------------|
| `==`     | `==`        |
| `!=`     | `!=`        |
| `>=`     | `>=`        |
| `<=`     | `<=`        |
| `>`      | `>`         |
| `<`      | `<`         |

**Regex combinada para operadores:**
```
(==|!=|>=|<=|>|<)
```

## Delimitadores

| Delimitador | Regex  |
|-------------|--------|
| `{`         | `\{`   |
| `}`         | `\}`   |
| `;`         | `;`    |

**Regex combinada para delimitadores:**
```
[\{\};]
```

## Palavras Reservadas (verificadas após identificar como identificador)
```
(device|sensor|pin|turn|on|off|wait|if|when|detected|not_detected)
```

## Multi-idioma (sinônimos em português)
```
(dispositivo|pino|ligar|desligar|esperar|se|quando|detectado|nao_detectado)
```

## Espaços em Branco (ignorados)
```
[ \t\r\n]+
```

## Comentários (linha)
```
//[^\n]*
```

## Token Inválido (qualquer caractere não reconhecido)
```
.
```
