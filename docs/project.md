# 🏠 HomeScript — Walkthrough do Projeto

## Resumo

Projeto completo de compilador para a linguagem **HomeScript**, voltada para automação residencial. O sistema traduz código `.iot` em C/Arduino funcional.

**Pipeline:** `.iot` → Tokens (Lexer) → AST (Parser) → Código C (CodeGen)

---

## O que foi construído

### 1. Compilador C Completo

O compilador processa código HomeScript em 3 fases:

- **Lexer** (`lexer/`): Identifica tokens — palavras reservadas, identificadores, números, operadores, delimitadores. Suporta português e inglês.
- **Parser** (`parser/`): Valida a gramática e constrói uma AST (Árvore Sintática Abstrata).
- **CodeGen** (`codegen/`): Converte a AST em código C/Arduino funcional com `setup()`, `loop()`, `pinMode`, `digitalWrite`, `digitalRead`, `analogRead`, `delay`.

**Uso na CLI:**
```bash
# Mostrar tudo (tokens + AST + código C)
./homescript exemplos/teste.iot

# Apenas código C
./homescript exemplos/teste.iot --code

# Apenas tokens
./homescript exemplos/teste.iot --tokens

# Saída JSON (para backend)
./homescript exemplos/teste.iot --json

# Salvar em arquivo
./homescript exemplos/teste.iot --output saida.c
```

### 2. Backend Python (FastAPI)

API REST com documentação automática em `/docs`:

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/compile` | POST | Compila HomeScript → tokens + AST + código C |
| `/tokens` | POST | Retorna apenas tokens |
| `/exemplos` | GET | Lista exemplos disponíveis |
| `/health` | GET | Status do servidor |

### 3. Frontend React (IDE)

Interface estilo IDE com tema escuro:

![Compilação gerando código C](C:\Users\joaop\.gemini\antigravity\brain\77ae49e8-3091-45b0-a9b9-47007e6de0c4\compilation_result.png)

![Visualização da tabela de tokens](C:\Users\joaop\.gemini\antigravity\brain\77ae49e8-3091-45b0-a9b9-47007e6de0c4\tokens_view.png)

![Interface visual para montar automações](C:\Users\joaop\.gemini\antigravity\brain\77ae49e8-3091-45b0-a9b9-47007e6de0c4\visual_builder.png)

---

## Como Executar

### 1. Compilar o compilador C
```bash
# Na raiz do projeto
C:\msys64\ucrt64\bin\gcc.exe -Wall -Wextra -std=c99 -o homescript.exe compiler/main.c lexer/lexer.c parser/parser.c parser/ast.c codegen/codegen.c
```

### 2. Iniciar o backend
```bash
cd backend
pip install -r requirements.txt
python app.py
# Servidor em http://localhost:8000
# Documentação em http://localhost:8000/docs
```

### 3. Iniciar o frontend
```bash
cd frontend
npm install
npm run dev
# Interface em http://localhost:5173
```

---

## Testes Realizados

| Teste | Resultado |
|-------|-----------|
| Compilação do compilador C | ✅ Sem erros |
| `teste.iot` → tokens | ✅ Todos os tokens identificados |
| `teste.iot` → AST | ✅ Árvore gerada corretamente |
| `teste.iot` → código C | ✅ Arduino válido gerado |
| `sensor_luz.iot` → código C | ✅ `analogRead` para sensores analógicos |
| `automacao_completa.iot` → código C | ✅ Múltiplos devices/sensors |
| Saída JSON do compilador | ✅ JSON válido para backend |
| Backend `/compile` | ✅ Retorna tokens + AST + código C |
| Frontend build | ✅ Sem erros de compilação |
| Frontend fluxo completo | ✅ Editor → Compilar → Resultado |
| Interface visual | ✅ Formulários funcionais |

---

## Funcionalidades Implementadas

- [x] Linguagem HomeScript definida com sintaxe clara
- [x] Tabela completa de tokens
- [x] Expressões regulares para cada tipo de token
- [x] Árvore hierárquica documentada
- [x] Analisador léxico em C (sem dependências externas)
- [x] 3 exemplos `.iot` funcionais
- [x] Explicação da linguagem (10 linhas)
- [x] Pitch técnico-comercial (4 slides)
- [x] Backend Python com FastAPI
- [x] Formato `.iot` documentado
- [x] Parser em C com construção de AST
- [x] Estruturas AST (Program, If, When, Command, Wait)
- [x] Gerador de código C/Arduino
- [x] Backend completo (tokens + AST + código C)
- [x] Frontend IDE React (editor + painel de resultado)
- [x] Interface visual (modo fácil)
- [x] Multi-idioma (português e inglês)
- [x] Validação de erros (token inválido, estrutura inválida, bloco não fechado)
- [x] Pipeline completo funcional (demonstração final)
