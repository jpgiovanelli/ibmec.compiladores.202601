## HomeScript (Home Automation Script) — Compilador `.iot` → C/Arduino

**Integrantes:**
- Enzo Perroni
- João Pedro Giovanelli
- Arthur Schiller
- Arthur Camaz
- Maria Claudia

Projeto de compiladores: linguagem **HomeScript** para automação residencial.
Pipeline: **`.iot` → Tokens (Lexer) → AST (Parser) → Código C/Arduino (CodeGen)**.

**Links**
- Documentação: https://www.overleaf.com/read/czsxgkvhzcmf#355f1e
- Vídeo: https://drive.google.com/file/d/141SZCPJO9-JFBybCya4G7Ga6OIb_sZDr/view?usp=sharing

### Começar rápido
- **Exemplos `.iot`**: [`exemplos/`](exemplos/)
  - [`exemplos/teste.iot`](exemplos/teste.iot)
  - [`exemplos/sensor_luz.iot`](exemplos/sensor_luz.iot)
  - [`exemplos/automacao_completa.iot`](exemplos/automacao_completa.iot)
- **Compilar o compilador (C)**: [`compiler/Makefile`](compiler/Makefile)
  - `make -C compiler`
- **Como usar o CLI** (flags `--tokens`, `--ast`, `--code`, `--output`, `--json`): [`compiler/main.c`](compiler/main.c)

### Documentação da linguagem
- **Definição da linguagem**: [`docs/definicao_linguagem.md`](docs/definicao_linguagem.md)
- **Formato do arquivo `.iot`**: [`docs/formato_iot.md`](docs/formato_iot.md)
- **Tabela de tokens**: [`docs/tabela_tokens.md`](docs/tabela_tokens.md)
- **Regex**: [`docs/regex.md`](docs/regex.md)
- **Árvore hierárquica (exemplos)**: [`docs/arvore_hierarquica.md`](docs/arvore_hierarquica.md)

### Sensores suportados (geração C automática)
- Genérico legado: `sensor nome pin <pino>;`
- DHT11: `sensor temperatura type dht11 pin 2;` (gera `#include <DHT.h>` e `begin()`)
- HC-SR04: `sensor distancia type hcsr04 trig 8 echo 9;` (gera leitura por `pulseIn`, sem biblioteca externa)
- Sintaxe PT-BR equivalente também suportada:
  - `sensor temperatura tipo dht11 pino 2;`
  - `sensor distancia tipo hcsr04 gatilho 8 eco 9;`

### Condicionais com `else`
- Inglês: `if/when ... { ... } else { ... }`
- PT-BR: `se/quando ... { ... } senao { ... }`

### Laços (`for` e `while`)
- Inglês:
  - `for i = 0; i < 10; i = i + 1 { ... }`
  - `while i < 10 { ... }`
- PT-BR:
  - `para i = 0; i < 10; i = i + 1 { ... }`
  - `enquanto i < 10 { ... }`

### Código-fonte (por fase do compilador)
- **Lexer**: [`lexer/`](lexer/)
- **Parser + AST**: [`parser/`](parser/)
- **Análise semântica**: [`semantic/`](semantic/)
- **Geração de código C/Arduino**: [`codegen/`](codegen/)
- **CLI principal**: [`compiler/main.c`](compiler/main.c)

### API e Frontend (opcional)
- **Backend (FastAPI)**: [`backend/`](backend/)
  - Endpoints e app: [`backend/app.py`](backend/app.py)
  - Bridge (executa o compilador + lê JSON): [`backend/compiler_bridge.py`](backend/compiler_bridge.py)
  - Dependências: [`backend/requirements.txt`](backend/requirements.txt)
- **Frontend (Vite + React + Monaco)**: [`frontend/`](frontend/)
  - README do frontend: [`frontend/README.md`](frontend/README.md)
  - Scripts e deps: [`frontend/package.json`](frontend/package.json)

### Walkthrough do projeto
- Visão geral e comandos: [`docs/project.md`](docs/project.md)
