Perfeito — vou te dar isso do jeito mais prático possível:
**instruções diretas para uma IA executar + roadmap depois.**

---

# 🧠 PARTE 1 — ENTREGA EM 2 SEMANAS (MÍNIMO DO PROFESSOR)

## 🎯 Objetivo

Construir um **pipeline funcional básico**:

```
.iot → tokens → (estrutura) → C
```

---
## 📌 Descrição do Projeto

O projeto consiste na criação da **HomeScript**, uma linguagem de programação própria voltada para automação residencial, que permite que usuários programem o comportamento de uma casa inteligente de forma simples e intuitiva.

A plataforma terá dois objetivos principais:

1. **Acadêmico:** desenvolver um compilador que interprete a linguagem HomeScript e a traduza para código em C, demonstrando conceitos de análise léxica, sintática e geração de código.

2. **Produto:** criar uma interface amigável (visual + código) onde o usuário possa montar automações e visualizar o código gerado, tendo a sensação de que está programando sua própria casa sem precisar conhecer C.

A entrada do sistema será um arquivo `.iot` (ou código digitado pelo usuário), que será processado e convertido em código C equivalente.
---

# 📦 TAREFAS (FORMATO PARA IA)

## 🔹 1. Definir a linguagem (estrutura + exemplo)

**Instrução para IA:**

> Crie a definição da linguagem HomeScript com:
>
> * Declaração de dispositivos (`device`, `sensor`)
> * Comandos (`turn on/off`, `wait`)
> * Estruturas (`if`, `when`)
> * Blocos com `{ }`
>   Gere 3 exemplos de código `.iot` simples usando luz, sensor e delay.

---

## 🔹 2. Tokens

**Instrução para IA:**

> Liste todos os tokens da linguagem incluindo:
>
> * palavras reservadas (device, sensor, when, if, turn, on, off, wait)
> * identificadores
> * números
> * operadores (`==`, `>`, `<`)
> * delimitadores (`{ } ;`)
>   Formate como tabela: Token | Tipo | Exemplo

---

## 🔹 3. Regex

**Instrução para IA:**

> Gere expressões regulares para:
>
> * identificadores
> * números
> * operadores
> * delimitadores
>   Não explique, apenas liste regex organizadas.

---

## 🔹 4. Árvore hierárquica

**Instrução para IA:**

> Dado este código:
>
> ```
> when movimento == detected {
>     turn luz on;
> }
> ```
>
> Gere a árvore hierárquica em formato textual (indentado).

---

## 🔹 5. Analisador léxico em C (OBRIGATÓRIO)

**Instrução para IA:**

> Gere um analisador léxico em C que:
>
> * leia um arquivo `.iot`
> * identifique tokens
> * imprima: tipo + valor
>   Não use bibliotecas externas.

---

## 🔹 6. Exemplo funcionando

**Instrução para IA:**

> Gere um arquivo `teste.iot` com:
>
> * declaração de dispositivo
> * um `when`
> * um `wait`
>   E mostre a saída esperada do lexer.

---

## 🔹 7. Estrutura da linguagem (explicação curta)

**Instrução para IA:**

> Explique em no máximo 10 linhas:
>
> * como a linguagem funciona
> * qual seu objetivo
> * como ela se relaciona com automação residencial

---

## 🔹 8. Apresentação técnico-comercial

**Instrução para IA:**

> Gere um pitch com:
>
> * problema
> * solução
> * diferencial
> * exemplo simples
>   Máximo 1 slide por tópico

---

## 🔹 9. Backend básico (OBRIGATÓRIO)

**Instrução para IA:**

> Crie um backend simples (Node ou Python) com:
>
> * endpoint `/compile`
> * recebe código HomeScript
> * retorna tokens (mock ou real)
>   Não precisa parser ainda.

---

## 🔹 10. Saída `.iot`

**Instrução para IA:**

> Defina que a entrada do sistema é um arquivo `.iot`
> e explique brevemente seu formato.

---

# ✅ RESULTADO DA ENTREGA

Você precisa conseguir mostrar:

* arquivo `.iot`
* lexer rodando em C
* tokens sendo gerados
* regex definida
* árvore (mesmo que manual)
* pitch do produto

👉 ISSO PASSA NA PRIMEIRA ENTREGA

---

# 🚀 PARTE 2 — EVOLUÇÃO DO PROJETO (PLATAFORMA COMPLETA)

## 🎯 Objetivo final

Transformar em produto com:

* linguagem funcional
* compilador completo
* frontend
* geração de C

---

# 📦 TAREFAS FUTURAS (FORMATO IA)

---

## 🔹 1. Parser

**Instrução para IA:**

> Implemente um parser em C que:
>
> * receba tokens
> * valide estrutura (`if`, `when`, blocos)
> * gere uma estrutura em memória (AST simples)

---

## 🔹 2. AST

**Instrução para IA:**

> Defina estruturas em C para representar:
>
> * Program
> * If
> * When
> * Command
> * Wait
>   Gere exemplo preenchido.

---

## 🔹 3. Code Generator (C)

**Instrução para IA:**

> Gere um módulo que converte AST para C:
>
> * `turn on` → `digitalWrite HIGH`
> * `wait` → `delay`
> * `if` → `if`
>   Retorne string final.

---

## 🔹 4. Integração backend

**Instrução para IA:**

> Atualize `/compile` para retornar:
>
> * tokens
> * AST
> * código C gerado

---

## 🔹 5. Frontend (editor)

**Instrução para IA:**

> Crie uma interface com:
>
> * textarea (HomeScript)
> * painel ao lado (C gerado)
> * botão "Compilar"
>   Use React simples.

---

## 🔹 6. Interface visual (modo fácil)

**Instrução para IA:**

> Crie um formulário que:
>
> * seleciona dispositivo
> * seleciona ação
> * seleciona gatilho
>   E gera automaticamente HomeScript.

---

## 🔹 7. Multi-idioma

**Instrução para IA:**

> Adapte o lexer para aceitar:
>
> * when / quando
> * if / se
> * turn / ligar
> * wait / esperar
>   Mapear tudo para tokens únicos.

---

## 🔹 8. Validação de erro

**Instrução para IA:**

> Adicione mensagens de erro:
>
> * token inválido
> * estrutura inválida
> * bloco não fechado

---

## 🔹 9. Demonstração final

**Instrução para IA:**

> Crie um fluxo completo:
>
> * usuário escreve código
> * sistema mostra tokens
> * sistema mostra árvore
> * sistema gera C

---

# 🧩 ORDEM REAL DE EXECUÇÃO

## Agora (2 semanas)

1. linguagem
2. tokens
3. regex
4. lexer em C
5. exemplo `.iot`
6. árvore (manual ok)
7. backend simples
8. pitch

---

## Depois

1. parser
2. AST
3. geração de C
4. frontend
5. integração
6. interface visual
7. polish final

---

# 🎯 RESUMO FINAL

## Entrega 1 = mostrar que vocês sabem:

* definir linguagem
* quebrar em tokens
* usar regex
* fazer lexer em C

## Projeto final = mostrar que vocês conseguem:

* compilar linguagem
* gerar código
* transformar isso em produto


