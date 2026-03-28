# 📄 Formato de Arquivo `.iot`

## Descrição

A entrada do sistema HomeScript é um arquivo com extensão **`.iot`** (Internet of Things). Este arquivo contém código escrito na linguagem HomeScript, que descreve o comportamento desejado para dispositivos de automação residencial.

## Formato

- **Extensão:** `.iot`
- **Codificação:** UTF-8
- **Tipo:** Texto puro (plain text)
- **Comentários:** Linhas iniciadas com `//` são ignoradas
- **Separador de comandos:** Ponto e vírgula `;`
- **Blocos:** Delimitados por `{ }`

## Estrutura Típica

Um arquivo `.iot` segue esta ordem:

1. Declarações de dispositivos (`device`)
2. Declarações de sensores (`sensor`)
3. Regras e comandos (`when`, `if`, `turn`, `wait`)

## Exemplo

```
// Automação de iluminação inteligente
device luz pin 13;
sensor movimento pin 2;

when movimento == detected {
    turn luz on;
    wait 5000;
    turn luz off;
}
```

## Processamento

O arquivo `.iot` é processado pelo compilador HomeScript em 3 etapas:

1. **Análise Léxica (Lexer):** Quebra o código em tokens
2. **Análise Sintática (Parser):** Valida a estrutura e gera AST
3. **Geração de Código:** Converte a AST em código C/Arduino equivalente
