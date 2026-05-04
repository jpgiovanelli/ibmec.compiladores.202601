# Storyboard do Front-end (HomeScript IDE)

## Objetivo
Documentar o front-end conforme implementação real: **2 telas principais** (modo Editor e modo Visual Builder), com a tela de Editor dividida em seções internas.

Referência de código:
- `frontend/src/App.jsx`
- `frontend/src/index.css`

---

## Visão geral de navegação
A aplicação usa um estado de modo:
- `modo = 'editor'` -> Tela principal da IDE textual.
- `modo = 'visual'` -> Tela de programação visual.

A troca ocorre pelos botões `Editor` e `Visual Builder` no cabeçalho.

---

## Tela 1 - IDE (modo Editor)
Esta é **uma única tela**, organizada em seções.

### Seção A - Cabeçalho (`header`)
**Elementos visíveis:**
- Logo `HomeScript IDE`
- Botões de modo: `Editor` / `Visual Builder`
- Indicador de status `Pronto`
- Dropdown `Exemplos` (3 exemplos prontos)

**Funcionalidades:**
1. Alternar entre as 2 telas da aplicação.
2. Carregar rapidamente um exemplo no editor.

### Seção B - Barra de compilação (`compile-bar`)
**Elementos visíveis:**
- Botão `Compilar (Ctrl/Cmd + Enter)`
- Estado de loading: `Compilando...`

**Funcionalidades:**
1. Disparar compilação via `POST /compile`.
2. Atalho de teclado para compilar.
3. Bloqueio de clique enquanto compila.

### Seção C - Área principal dividida (`main-content`)
Layout com split horizontal, ajustável pelo usuário.

#### C1. Painel esquerdo: Editor (`editor-panel`)
**Elementos visíveis:**
- Título `homescript (.iot)`
- Monaco Editor com tema customizado

**Funcionalidades:**
1. Edição de código HomeScript.
2. Syntax highlight para keywords, números, operadores e comentários.
3. Auto-complete com snippets da linguagem.
4. Sugestão de identificadores já declarados no código.
5. Redimensionamento da largura do painel (20% a 80%) pelo `panel-resizer`.

#### C2. Painel direito: Resultados (`result-panel`)
**Elementos visíveis:**
- Abas: `Código C`, `Execução`, `Tokens`, `AST`

**Funcionalidades por aba:**
1. `Código C`: mostra saída C/Arduino gerada.
2. `Execução`: mostra trace passo a passo do pipeline (entrada, léxico, sintático, semântico, codegen), com `Anterior`, `Próximo` e `Auto Play`.
3. `Tokens`: tabela com linha, coluna, tipo e valor dos tokens.
4. `AST`: árvore sintática textual.

**Tratamento de erro (na mesma seção):**
- Se falhar compilação, exibe `error-box` com erro principal e lista detalhada (`fase`, `linha`, `coluna`, `mensagem`).

---

## Tela 2 - Programação Visual (modo Visual Builder)
Tela dedicada à montagem de automações sem digitar código.

### Seção A - Cadastro de dispositivos e sensores
**Elementos visíveis:**
- Lista dinâmica de cards com:
  - tipo (`device` ou `sensor`)
  - nome
  - pino
- Botão `+ Adicionar dispositivo`
- Botão de remoção `✕` (quando há mais de um item)

**Funcionalidades:**
1. Adicionar/remover itens.
2. Definir inventário base da automação.

### Seção B - Regras de automação
**Elementos visíveis:**
- Lista dinâmica de cards de regra com:
  - gatilho (sensor)
  - operador (`==`, `!=`, `>`, `<`)
  - valor
  - ação (dispositivo)
  - estado (`ON`/`OFF`)
  - `wait` opcional
- Botão `+ Adicionar regra`
- Botão de remoção `✕`

**Funcionalidades:**
1. Montar lógica `when ... { turn ... ; wait ...; }`.
2. Relacionar sensores e dispositivos já cadastrados.

### Seção C - Geração de código
**Elemento visível:**
- Botão `Gerar Código HomeScript`

**Funcionalidades:**
1. Gera texto HomeScript automaticamente.
2. Envia o código para a tela Editor.
3. Retorna automaticamente para `modo = 'editor'`.

---

## Lista consolidada de funcionalidades do front-end
1. Alternância entre duas telas (`Editor` e `Visual Builder`).
2. Editor Monaco com tema e configuração de linguagem HomeScript.
3. Auto-complete com snippets e sugestões contextuais.
4. Carregamento de exemplos prontos.
5. Compilação via API FastAPI.
6. Painel de resultados com 4 abas (`Código C`, `Execução`, `Tokens`, `AST`).
7. Trace de execução com navegação manual e autoplay.
8. Exibição detalhada de erros por fase e posição.
9. Layout com divisor arrastável entre editor e resultados.
10. Builder visual para gerar código HomeScript sem escrita manual.

---

## Jornada real do usuário
1. Usuário abre a tela Editor.
2. Escreve código ou carrega exemplo.
3. Compila e analisa resultados nas abas.
4. Se quiser, troca para Visual Builder.
5. Monta automação visualmente e gera código.
6. Retorna ao Editor, recompila e valida saída final.
