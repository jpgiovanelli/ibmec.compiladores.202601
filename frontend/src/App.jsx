import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import Editor from '@monaco-editor/react'
import './index.css'

/* ===== Exemplos de código HomeScript ===== */
const EXEMPLOS = [
  {
    nome: 'Luz com Sensor',
    codigo: `// Automação de iluminação com sensor de movimento
device luz pin 13;
sensor movimento pin 2;

when movimento == detected {
    turn luz on;
    wait 5000;
    turn luz off;
}`
  },
  {
    nome: 'Ventilador com Temperatura',
    codigo: `// Controle de ventilador por temperatura
device ventilador pin 7;
sensor temperatura pin A0;

if temperatura > 30 {
    turn ventilador on;
}

if temperatura < 25 {
    turn ventilador off;
}`
  },
  {
    nome: 'Automação Completa',
    codigo: `// Automação completa com múltiplos dispositivos
device luz_sala pin 13;
device luz_cozinha pin 12;
device ventilador pin 7;
device alarme pin 6;

sensor movimento pin 2;
sensor temperatura pin A0;
sensor porta pin 3;

when movimento == detected {
    turn luz_sala on;
    turn luz_cozinha on;
    wait 10000;
    turn luz_sala off;
    turn luz_cozinha off;
}

if temperatura > 28 {
    turn ventilador on;
}

if temperatura < 22 {
    turn ventilador off;
}

when porta == detected {
    turn alarme on;
    wait 3000;
    turn alarme off;
}`
  }
]

/* ===== API helper ===== */
const API_URL = 'http://localhost:8000'
let homescriptConfigured = false

function configurarHomeScript(monaco) {
  if (homescriptConfigured) return
  homescriptConfigured = true

  monaco.languages.register({ id: 'homescript' })

  monaco.languages.setMonarchTokensProvider('homescript', {
    tokenizer: {
      root: [
        [/\/\/.*$/, 'comment'],
        [/\b(device|sensor|pin|let|print|turn|on|off|wait|if|when|detected|not_detected)\b/, 'keyword'],
        [/\b(dispositivo|pino|ligar|desligar|esperar|se|quando|detectado|nao_detectado)\b/, 'keyword'],
        [/\bA[0-9]+\b/, 'number'],
        [/\b[0-9]+\b/, 'number'],
        [/[{}()[\]]/, '@brackets'],
        [/==|!=|>=|<=|>|<|=|\+|-|\*|\//, 'operator'],
        [/[;,.]/, 'delimiter'],
        [/[a-zA-Z_][a-zA-Z0-9_]*/, 'identifier']
      ]
    }
  })

  monaco.languages.setLanguageConfiguration('homescript', {
    comments: { lineComment: '//' },
    brackets: [['{', '}'], ['(', ')']],
    autoClosingPairs: [
      { open: '{', close: '}' },
      { open: '(', close: ')' }
    ],
    surroundingPairs: [
      { open: '{', close: '}' },
      { open: '(', close: ')' }
    ]
  })

  monaco.editor.defineTheme('homescript-ide', {
    base: 'vs-dark',
    inherit: true,
    rules: [
      { token: 'keyword', foreground: 'C586C0' },
      { token: 'comment', foreground: '6A9955' },
      { token: 'number', foreground: 'B5CEA8' },
      { token: 'operator', foreground: 'D4D4D4' },
      { token: 'identifier', foreground: '9CDCFE' }
    ],
    colors: {
      'editor.background': '#1e1e1e',
      'editorLineNumber.foreground': '#6b7280',
      'editorLineNumber.activeForeground': '#c9d1d9',
      'editorGutter.background': '#1e1e1e',
      'editorCursor.foreground': '#d4d4d4',
      'editor.selectionBackground': '#264f78',
      'editor.inactiveSelectionBackground': '#3a3d41',
      'editorIndentGuide.background1': '#2a2d2e'
    }
  })

  monaco.languages.registerCompletionItemProvider('homescript', {
    triggerCharacters: [' ', '_'],
    provideCompletionItems: (model, position) => {
      const word = model.getWordUntilPosition(position)
      const range = {
        startLineNumber: position.lineNumber,
        endLineNumber: position.lineNumber,
        startColumn: word.startColumn,
        endColumn: word.endColumn
      }

      const snippet = monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet
      const k = monaco.languages.CompletionItemKind
      const sugestoes = [
        { label: 'device', insertText: 'device ${1:nome} pin ${2:13};', detail: 'Declarar dispositivo', kind: k.Keyword, insertTextRules: snippet },
        { label: 'dispositivo', insertText: 'dispositivo ${1:nome} pino ${2:13};', detail: 'Declarar dispositivo (PT-BR)', kind: k.Keyword, insertTextRules: snippet },
        { label: 'sensor', insertText: 'sensor ${1:nome} pin ${2:A0};', detail: 'Declarar sensor', kind: k.Keyword, insertTextRules: snippet },
        { label: 'pin', insertText: 'pin', detail: 'Palavra-chave de pino (EN)', kind: k.Keyword },
        { label: 'pino', insertText: 'pino', detail: 'Palavra-chave de pino (PT-BR)', kind: k.Keyword },
        { label: 'let', insertText: 'let ${1:variavel} = ${2:0};', detail: 'Declarar variável', kind: k.Keyword, insertTextRules: snippet },
        { label: 'turn on', insertText: 'turn ${1:dispositivo} on;', detail: 'Ligar dispositivo', kind: k.Snippet, insertTextRules: snippet },
        { label: 'turn off', insertText: 'turn ${1:dispositivo} off;', detail: 'Desligar dispositivo', kind: k.Snippet, insertTextRules: snippet },
        { label: 'when', insertText: 'when ${1:sensor} == ${2:detected} {\n\t$0\n}', detail: 'Regra when', kind: k.Keyword, insertTextRules: snippet },
        { label: 'if', insertText: 'if ${1:sensor} > ${2:0} {\n\t$0\n}', detail: 'Condição if', kind: k.Keyword, insertTextRules: snippet },
        { label: 'wait', insertText: 'wait ${1:1000};', detail: 'Aguardar em ms', kind: k.Keyword, insertTextRules: snippet },
        { label: 'print', insertText: 'print ${1:valor};', detail: 'Impressão serial', kind: k.Keyword, insertTextRules: snippet },
        { label: 'detected', insertText: 'detected', detail: 'Estado detectado', kind: k.Constant },
        { label: 'not_detected', insertText: 'not_detected', detail: 'Estado não detectado', kind: k.Constant },
        { label: 'detectado', insertText: 'detectado', detail: 'Estado detectado (PT-BR)', kind: k.Constant },
        { label: 'nao_detectado', insertText: 'nao_detectado', detail: 'Estado não detectado (PT-BR)', kind: k.Constant },
        { label: 'ligar', insertText: 'ligar ${1:dispositivo};', detail: 'Atalho PT-BR para ligar', kind: k.Keyword, insertTextRules: snippet },
        { label: 'desligar', insertText: 'desligar ${1:dispositivo};', detail: 'Atalho PT-BR para desligar', kind: k.Keyword, insertTextRules: snippet },
        { label: 'esperar', insertText: 'esperar ${1:1000};', detail: 'Atalho PT-BR para wait', kind: k.Keyword, insertTextRules: snippet },
        { label: 'quando', insertText: 'quando ${1:sensor} == ${2:detectado} {\n\t$0\n}', detail: 'Atalho PT-BR para when', kind: k.Keyword, insertTextRules: snippet },
        { label: 'se', insertText: 'se ${1:sensor} > ${2:0} {\n\t$0\n}', detail: 'Atalho PT-BR para if', kind: k.Keyword, insertTextRules: snippet }
      ].map((item) => ({ ...item, range }))

      const simbolos = new Set()
      const texto = model.getValue()
      const regex = /\b(?:device|sensor|let)\s+([a-zA-Z_][a-zA-Z0-9_]*)\b/g
      let match = regex.exec(texto)
      while (match) {
        simbolos.add(match[1])
        match = regex.exec(texto)
      }

      for (const simbolo of simbolos) {
        sugestoes.push({
          label: simbolo,
          kind: k.Variable,
          detail: 'Identificador declarado',
          insertText: simbolo,
          range
        })
      }

      return { suggestions: sugestoes }
    }
  })
}

async function compilar(codigo) {
  const res = await fetch(`${API_URL}/compile`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ codigo })
  })
  return res.json()
}

function resumirNoAst(node) {
  const partes = [node.tipo]
  if (node.nome) partes.push(`nome=${node.nome}`)
  if (node.pino) partes.push(`pino=${node.pino}`)
  if (node.operador) partes.push(`op=${node.operador}`)
  if (node.valor) partes.push(`valor=${node.valor}`)
  if (node.estado) partes.push(`estado=${node.estado}`)
  if (node.tempo !== undefined) partes.push(`tempo=${node.tempo}`)
  if (node.expressao) partes.push(`expr=${node.expressao}`)
  return partes.join(' | ')
}

function coletarNosAst(node, lista = []) {
  if (!node) return lista
  if (node.tipo !== 'PROGRAM') {
    lista.push(node)
  }
  if (Array.isArray(node.filhos)) {
    node.filhos.forEach((filho) => coletarNosAst(filho, lista))
  }
  return lista
}

const LEXER_SNIPPETS = {
  fluxo: `Token lexer_proximo_token(Lexer *lexer) {
  lexer_pular_insignificantes(lexer);
  if (lexer->eof) return TOKEN_EOF;

  char c = lexer->caractere_atual;
  if (c == 'A' && isdigit(lexer_espiar(lexer))) return lexer_ler_pino_analogico(lexer);
  if (isalpha(c) || c == '_') return lexer_ler_identificador(lexer);
  if (isdigit(c)) return lexer_ler_numero(lexer);
  if (c == '=' || c == '!' || c == '>' || c == '<') return lexer_ler_operador(lexer);
  /* + - * / { } ; ( ) ... */
}`,
  identificador: `static Token lexer_ler_identificador(Lexer *lexer) {
  while (!lexer->eof && (isalnum(lexer->caractere_atual) || lexer->caractere_atual == '_')) {
    buffer[i++] = lexer->caractere_atual;
    lexer_avancar(lexer);   // avanca caractere por caractere
  }
  TokenType tipo = verificar_palavra_reservada(buffer);
  return criar_token(tipo, buffer, linha_inicio, coluna_inicio);
}`,
  numero: `static Token lexer_ler_numero(Lexer *lexer) {
  while (!lexer->eof && isdigit(lexer->caractere_atual)) {
    buffer[i++] = lexer->caractere_atual;
    lexer_avancar(lexer);   // avanca 1 caractere
  }
  return criar_token(TOKEN_NUMBER, buffer, linha_inicio, coluna_inicio);
}`,
  pinoAnalogico: `static Token lexer_ler_pino_analogico(Lexer *lexer) {
  buffer[i++] = lexer->caractere_atual; // 'A'
  lexer_avancar(lexer);
  while (!lexer->eof && isdigit(lexer->caractere_atual)) {
    buffer[i++] = lexer->caractere_atual;
    lexer_avancar(lexer);
  }
  return criar_token(TOKEN_ANALOG_PIN, buffer, linha_inicio, coluna_inicio);
}`,
  operadorRelacional: `static Token lexer_ler_operador(Lexer *lexer) {
  switch (lexer->caractere_atual) {
    case '=': if (lexer_espiar(lexer) == '=') return TOKEN_OP_EQUAL; else return TOKEN_OP_ASSIGN;
    case '!': if (lexer_espiar(lexer) == '=') return TOKEN_OP_NOT_EQUAL; else return TOKEN_ERROR;
    case '>': if (lexer_espiar(lexer) == '=') return TOKEN_OP_GREATER_EQUAL; else return TOKEN_OP_GREATER;
    case '<': if (lexer_espiar(lexer) == '=') return TOKEN_OP_LESS_EQUAL; else return TOKEN_OP_LESS;
  }
}`,
  delimitadorOuAritmetico: `if (c == '+') return TOKEN_OP_PLUS;
if (c == '-') return TOKEN_OP_MINUS;
if (c == '*') return TOKEN_OP_MULT;
if (c == '/') return TOKEN_OP_DIV;
if (c == '{') return TOKEN_LBRACE;
if (c == '}') return TOKEN_RBRACE;
if (c == ';') return TOKEN_SEMICOLON;
if (c == '(') return TOKEN_LPAREN;
if (c == ')') return TOKEN_RPAREN;`,
  erro: `/* Token invalido */
char erro[2] = {c, '\\0'};
lexer_avancar(lexer);
return criar_token(TOKEN_ERROR, erro, linha, coluna);`,
  avancar: `static void lexer_avancar(Lexer *lexer) {
  lexer->caractere_atual = fgetc(lexer->arquivo);
  if (lexer->caractere_atual == '\\n') { lexer->linha++; lexer->coluna = 0; }
  else { lexer->coluna++; }
}`
}

function montarRastroCaracteres(token) {
  if (!token || !token.valor || token.valor === 'EOF') return []
  const inicio = typeof token.coluna === 'number' ? token.coluna : 1
  return Array.from(token.valor).map((caractere, index) => ({
    indice: index + 1,
    caractere,
    coluna: inicio + index
  }))
}

function mapearDebugLexico(token) {
  if (!token) {
    return {
      funcao: 'lexer_proximo_token',
      snippet: LEXER_SNIPPETS.fluxo,
      rastro: []
    }
  }

  const tipo = token.tipo || ''
  if (tipo === 'IDENTIFIER' || tipo.startsWith('KEYWORD_')) {
    return {
      funcao: 'lexer_proximo_token -> lexer_ler_identificador -> verificar_palavra_reservada',
      snippet: `${LEXER_SNIPPETS.identificador}\n\n${LEXER_SNIPPETS.avancar}`,
      rastro: montarRastroCaracteres(token)
    }
  }
  if (tipo === 'NUMBER') {
    return {
      funcao: 'lexer_proximo_token -> lexer_ler_numero',
      snippet: `${LEXER_SNIPPETS.numero}\n\n${LEXER_SNIPPETS.avancar}`,
      rastro: montarRastroCaracteres(token)
    }
  }
  if (tipo === 'ANALOG_PIN') {
    return {
      funcao: 'lexer_proximo_token -> lexer_ler_pino_analogico',
      snippet: `${LEXER_SNIPPETS.pinoAnalogico}\n\n${LEXER_SNIPPETS.avancar}`,
      rastro: montarRastroCaracteres(token)
    }
  }
  if (tipo.startsWith('OP_EQUAL') || tipo.startsWith('OP_ASSIGN') || tipo.startsWith('OP_NOT') || tipo.startsWith('OP_GREATER') || tipo.startsWith('OP_LESS')) {
    return {
      funcao: 'lexer_proximo_token -> lexer_ler_operador',
      snippet: `${LEXER_SNIPPETS.operadorRelacional}\n\n${LEXER_SNIPPETS.avancar}`,
      rastro: montarRastroCaracteres(token)
    }
  }
  if (tipo.startsWith('OP_') || tipo.startsWith('DELIM_')) {
    return {
      funcao: 'lexer_proximo_token (bloco de operadores/delimitadores)',
      snippet: `${LEXER_SNIPPETS.delimitadorOuAritmetico}\n\n${LEXER_SNIPPETS.avancar}`,
      rastro: montarRastroCaracteres(token)
    }
  }

  return {
    funcao: 'lexer_proximo_token (fallback de erro)',
    snippet: `${LEXER_SNIPPETS.erro}\n\n${LEXER_SNIPPETS.avancar}`,
    rastro: montarRastroCaracteres(token)
  }
}

function buildExecutionTrace(codigo, resultado) {
  const linhasFonte = codigo.split('\n')
  const steps = []

  steps.push({
    stage: 'entrada',
    title: 'Entrada do compilador',
    detail: `O compilador recebe ${linhasFonte.length} linha(s) de texto HomeScript.`,
    output: 'Objeto de entrada com o código bruto.'
  })

  linhasFonte.forEach((linha, index) => {
    const texto = linha.trim()
    if (!texto || texto.startsWith('//')) return
    steps.push({
      stage: 'leitura',
      line: index + 1,
      title: `Leitura da linha ${index + 1}`,
      detail: `Texto lido: "${texto}"`,
      output: 'A linha segue para o analisador léxico.'
    })
  })

  const tokens = Array.isArray(resultado?.tokens) ? resultado.tokens : []
  const tokensSemEof = tokens.filter((token) => token.tipo !== 'EOF')
  tokensSemEof.forEach((token, index) => {
    const debug = mapearDebugLexico(token)
    steps.push({
      stage: 'lexico',
      line: token.linha,
      title: `Token ${index + 1} de ${tokensSemEof.length}`,
      detail: `Lexema "${token.valor}" foi classificado como ${token.tipo}.`,
      output: `Token = { tipo: ${token.tipo}, valor: "${token.valor}", linha: ${token.linha}, coluna: ${token.coluna} }`,
      debug
    })
  })

  if (tokens.length > 0) {
    steps.push({
      stage: 'lexico',
      title: 'Fim da análise léxica',
      detail: 'Todos os tokens foram identificados e ordenados.',
      output: `Fluxo total: ${tokens.length} token(s), incluindo EOF.`
    })
  }

  const nosAst = coletarNosAst(resultado?.ast)
  nosAst.forEach((no, index) => {
    steps.push({
      stage: 'sintatico',
      line: no.linha,
      title: `Nó AST ${index + 1} de ${nosAst.length}`,
      detail: `Parser consumiu tokens e montou o nó ${no.tipo}.`,
      output: resumirNoAst(no)
    })
  })

  if (nosAst.length > 0) {
    steps.push({
      stage: 'sintatico',
      title: 'Fim da análise sintática',
      detail: 'A sequência de tokens foi organizada em árvore (AST).',
      output: `AST com ${nosAst.length} nó(s) úteis.`
    })
  }

  if (resultado?.sucesso) {
    steps.push({
      stage: 'semantico',
      title: 'Análise semântica',
      detail: 'Verificação de consistência (símbolos, uso válido e regras da linguagem).',
      output: 'Sem erros semânticos.'
    })
  } else {
    const erros = Array.isArray(resultado?.erros) ? resultado.erros : []
    if (erros.length === 0) {
      steps.push({
        stage: 'semantico',
        title: 'Análise interrompida',
        detail: resultado?.erro || 'Falha de compilação.',
        output: 'Compilação finalizou com erro.'
      })
    } else {
      erros.forEach((erro, index) => {
        steps.push({
          stage: 'semantico',
          line: erro.linha,
          title: `Erro ${index + 1} de ${erros.length}`,
          detail: `[${erro.fase || 'erro'}] ${erro.mensagem}`,
          output: `Linha ${erro.linha ?? '-'}, coluna ${erro.coluna ?? '-'}`
        })
      })
    }
  }

  const codigoGerado = resultado?.codigo_c || ''
  if (codigoGerado) {
    const linhasGeradas = codigoGerado.split('\n')
    steps.push({
      stage: 'codegen',
      title: 'Início da geração de código',
      detail: 'O gerador percorre a AST e escreve o código C final.',
      output: `Saída com ${linhasGeradas.length} linha(s).`
    })

    linhasGeradas.forEach((linha, index) => {
      const texto = linha.trim()
      if (!texto) return
      steps.push({
        stage: 'codegen',
        title: `Linha C ${index + 1}`,
        detail: `Trecho gerado: "${texto}"`,
        output: 'Linha adicionada ao arquivo de saída C.'
      })
    })
  }

  return { linhasFonte, steps }
}

function CodeBlock({ text }) {
  const lines = String(text || '').split('\n')
  return (
    <div className="trace-codeblock">
      {lines.map((line, idx) => (
        <div key={idx} className="trace-codeblock-line">
          <span className="trace-codeblock-ln">{String(idx + 1).padStart(2, '0')}</span>
          <span className="trace-codeblock-text">{line || ' '}</span>
        </div>
      ))}
    </div>
  )
}

/* ===== Componente: TokensView ===== */
function TokensView({ tokens }) {
  if (!tokens || tokens.length === 0) {
    return (
      <div className="empty-state">
        <div className="empty-icon">🔤</div>
        <div className="empty-text">Compile o código para ver os tokens</div>
      </div>
    )
  }

  return (
    <table className="tokens-table">
      <thead>
        <tr>
          <th>Linha</th>
          <th>Coluna</th>
          <th>Tipo</th>
          <th>Valor</th>
        </tr>
      </thead>
      <tbody>
        {tokens.map((t, i) => (
          <tr key={i} className="fade-in" style={{ animationDelay: `${i * 20}ms` }}>
            <td className="token-pos">{t.linha}</td>
            <td className="token-pos">{t.coluna}</td>
            <td className="token-type">{t.tipo}</td>
            <td className="token-value">{t.valor}</td>
          </tr>
        ))}
      </tbody>
    </table>
  )
}

/* ===== Componente: ASTView ===== */
function ASTView({ ast }) {
  if (!ast) {
    return (
      <div className="empty-state">
        <div className="empty-icon">🌳</div>
        <div className="empty-text">Compile o código para ver a árvore sintática</div>
      </div>
    )
  }

  function renderNode(node, indent = 0) {
    if (!node) return ''
    const prefix = indent === 0 ? '' : '  '.repeat(indent - 1) + (indent > 0 ? '├── ' : '')
    let details = ''

    if (node.nome) details += ` nome: ${node.nome}`
    if (node.pino) details += `, pino: ${node.pino}`
    if (node.operador) details += ` ${node.operador}`
    if (node.valor) details += ` ${node.valor}`
    if (node.estado) details += ` estado: ${node.estado}`
    if (node.tempo) details += ` tempo: ${node.tempo}ms`

    let result = prefix + node.tipo
    if (details) result += ' (' + details.trim() + ')'
    result += '\n'

    if (node.filhos) {
      node.filhos.forEach(child => {
        result += renderNode(child, indent + 1)
      })
    }

    return result
  }

  return (
    <pre className="ast-tree fade-in">{renderNode(ast)}</pre>
  )
}

/* ===== Componente: CodeView ===== */
function CodeView({ code }) {
  if (!code) {
    return (
      <div className="empty-state">
        <div className="empty-icon">⚡</div>
        <div className="empty-text">Compile o código para ver o C/Arduino gerado</div>
      </div>
    )
  }

  return (
    <pre className="code-output fade-in">{code}</pre>
  )
}

function ExecutionTraceView({ codigo, resultado }) {
  const [currentStep, setCurrentStep] = useState(0)
  const [playing, setPlaying] = useState(false)
  const trace = useMemo(() => buildExecutionTrace(codigo, resultado), [codigo, resultado])
  const step = trace.steps[currentStep]

  useEffect(() => {
    if (!playing) return
    if (trace.steps.length === 0) return

    const timer = setInterval(() => {
      setCurrentStep((prev) => {
        if (prev >= trace.steps.length - 1) {
          setPlaying(false)
          return prev
        }
        return prev + 1
      })
    }, 1200)

    return () => clearInterval(timer)
  }, [playing, trace.steps.length])

  if (!resultado) {
    return (
      <div className="empty-state">
        <div className="empty-icon">🧭</div>
        <div className="empty-text">Compile para ver a execução linha a linha</div>
      </div>
    )
  }

  if (trace.steps.length === 0) {
    return (
      <div className="empty-state">
        <div className="empty-icon">🧭</div>
        <div className="empty-text">Nenhuma etapa disponível para este código</div>
      </div>
    )
  }

  return (
    <div className="trace-layout fade-in">
      <div className="trace-toolbar">
        <button
          className="trace-btn"
          onClick={() => setCurrentStep((prev) => Math.max(prev - 1, 0))}
          disabled={currentStep === 0}
        >
          Anterior
        </button>
        <button
          className="trace-btn trace-btn-primary"
          onClick={() => setPlaying((prev) => !prev)}
        >
          {playing ? 'Pausar' : 'Auto Play'}
        </button>
        <button
          className="trace-btn"
          onClick={() => setCurrentStep((prev) => Math.min(prev + 1, trace.steps.length - 1))}
          disabled={currentStep >= trace.steps.length - 1}
        >
          Próximo
        </button>
        <div className="trace-progress">
          Etapa {currentStep + 1} / {trace.steps.length}
        </div>
      </div>

      <div className="trace-grid">
        <div className="trace-source">
          <div className="trace-card-title">Código de Entrada</div>
          <pre className="trace-code">
            {trace.linhasFonte.map((linha, index) => {
              const lineNumber = index + 1
              const active = step?.line === lineNumber
              return (
                <div key={lineNumber} className={`trace-line ${active ? 'active' : ''}`}>
                  <span className="trace-ln">{String(lineNumber).padStart(2, '0')}</span>
                  <span>{linha || ' '}</span>
                </div>
              )
            })}
          </pre>
        </div>

        <div className="trace-details">
          <div className="trace-card-title">Estado da Execução</div>
          <div className="trace-stage">{step.stage.toUpperCase()}</div>
          <div className="trace-title">{step.title}</div>
          <div className="trace-detail">{step.detail}</div>
          <div className="trace-output-label">Saída da etapa</div>
          <pre className="trace-output">{step.output}</pre>
          {step.stage === 'lexico' && step.debug && (
            <>
              <div className="trace-output-label">Função interna usada</div>
              <CodeBlock text={step.debug.funcao} />
              <div className="trace-output-label">Código interno do lexer</div>
              <CodeBlock text={step.debug.snippet} />
              <div className="trace-output-label">Consumo caractere por caractere</div>
              <div className="trace-char-list">
                {(step.debug.rastro || []).length === 0 && (
                  <div className="trace-char-empty">Sem rastro para este token</div>
                )}
                {(step.debug.rastro || []).map((item) => (
                  <div key={`${item.indice}-${item.coluna}`} className="trace-char-item">
                    <span className="trace-char-index">#{item.indice}</span>
                    <span className="trace-char-value">'{item.caractere}'</span>
                    <span className="trace-char-col">coluna {item.coluna}</span>
                  </div>
                ))}
              </div>
            </>
          )}
        </div>
      </div>
    </div>
  )
}

/* ===== Componente: VisualBuilder ===== */
function VisualBuilder({ onGenerate }) {
  const [devices, setDevices] = useState([{ nome: '', pino: '', tipo: 'device' }])
  const [regras, setRegras] = useState([{ gatilho: '', operador: '==', valor: 'detected', acao: '', estado: 'on', wait: '' }])

  const addDevice = () => setDevices([...devices, { nome: '', pino: '', tipo: 'device' }])
  const removeDevice = (i) => setDevices(devices.filter((_, idx) => idx !== i))
  const updateDevice = (i, field, val) => {
    const updated = [...devices]
    updated[i] = { ...updated[i], [field]: val }
    setDevices(updated)
  }

  const addRegra = () => setRegras([...regras, { gatilho: '', operador: '==', valor: 'detected', acao: '', estado: 'on', wait: '' }])
  const removeRegra = (i) => setRegras(regras.filter((_, idx) => idx !== i))
  const updateRegra = (i, field, val) => {
    const updated = [...regras]
    updated[i] = { ...updated[i], [field]: val }
    setRegras(updated)
  }

  const gerarCodigo = () => {
    let code = '// Código gerado pelo HomeScript Visual Builder\n'

    devices.forEach(d => {
      if (d.nome && d.pino) {
        code += `${d.tipo} ${d.nome} pin ${d.pino};\n`
      }
    })

    code += '\n'

    regras.forEach(r => {
      if (r.gatilho && r.acao) {
        code += `when ${r.gatilho} ${r.operador} ${r.valor} {\n`
        code += `    turn ${r.acao} ${r.estado};\n`
        if (r.wait) {
          code += `    wait ${r.wait};\n`
        }
        code += '}\n\n'
      }
    })

    onGenerate(code.trim())
  }

  const sensorNames = devices.filter(d => d.tipo === 'sensor' && d.nome).map(d => d.nome)
  const deviceNames = devices.filter(d => d.tipo === 'device' && d.nome).map(d => d.nome)

  return (
    <div className="visual-builder">
      {/* Dispositivos */}
      <div className="builder-section">
        <div className="builder-section-title">📟 Dispositivos e Sensores</div>
        {devices.map((d, i) => (
          <div key={i} className="rule-card">
            <select className="builder-select" value={d.tipo} onChange={e => updateDevice(i, 'tipo', e.target.value)}>
              <option value="device">Dispositivo</option>
              <option value="sensor">Sensor</option>
            </select>
            <input className="builder-input" placeholder="Nome (ex: luz)" value={d.nome}
              onChange={e => updateDevice(i, 'nome', e.target.value)} />
            <span style={{ color: 'var(--text-muted)', fontSize: 13 }}>pin</span>
            <input className="builder-input" placeholder="Pino (ex: 13)" value={d.pino}
              onChange={e => updateDevice(i, 'pino', e.target.value)} style={{ width: 100 }} />
            {devices.length > 1 && (
              <button className="rule-remove" onClick={() => removeDevice(i)}>✕</button>
            )}
          </div>
        ))}
        <button className="add-btn" onClick={addDevice}>+ Adicionar dispositivo</button>
      </div>

      {/* Regras */}
      <div className="builder-section">
        <div className="builder-section-title">⚡ Regras de Automação</div>
        {regras.map((r, i) => (
          <div key={i} className="rule-card">
            <span style={{ color: 'var(--accent-orange)', fontWeight: 600, fontSize: 13 }}>QUANDO</span>
            <select className="builder-select" value={r.gatilho} onChange={e => updateRegra(i, 'gatilho', e.target.value)}>
              <option value="">Selecionar sensor...</option>
              {sensorNames.map(s => <option key={s} value={s}>{s}</option>)}
            </select>
            <select className="builder-select" value={r.operador} onChange={e => updateRegra(i, 'operador', e.target.value)} style={{ width: 80 }}>
              <option value="==">==</option>
              <option value="!=">!=</option>
              <option value=">">&gt;</option>
              <option value="<">&lt;</option>
            </select>
            <input className="builder-input" placeholder="Valor" value={r.valor}
              onChange={e => updateRegra(i, 'valor', e.target.value)} style={{ width: 120 }} />
            <span style={{ color: 'var(--accent-green)', fontWeight: 600, fontSize: 13 }}>→</span>
            <select className="builder-select" value={r.acao} onChange={e => updateRegra(i, 'acao', e.target.value)}>
              <option value="">Selecionar dispositivo...</option>
              {deviceNames.map(d => <option key={d} value={d}>{d}</option>)}
            </select>
            <select className="builder-select" value={r.estado} onChange={e => updateRegra(i, 'estado', e.target.value)} style={{ width: 80 }}>
              <option value="on">ON</option>
              <option value="off">OFF</option>
            </select>
            <input className="builder-input" placeholder="Wait (ms)" value={r.wait}
              onChange={e => updateRegra(i, 'wait', e.target.value)} style={{ width: 120 }} />
            {regras.length > 1 && (
              <button className="rule-remove" onClick={() => removeRegra(i)}>✕</button>
            )}
          </div>
        ))}
        <button className="add-btn" onClick={addRegra}>+ Adicionar regra</button>
      </div>

      {/* Gerar */}
      <div style={{ display: 'flex', justifyContent: 'center' }}>
        <button className="generate-btn" onClick={gerarCodigo}>
          🔧 Gerar Código HomeScript
        </button>
      </div>
    </div>
  )
}

/* ===== App Principal ===== */
function App() {
  const [modo, setModo] = useState('editor') // 'editor' | 'visual'
  const [codigo, setCodigo] = useState(EXEMPLOS[0].codigo)
  const [resultado, setResultado] = useState(null)
  const [abaResultado, setAbaResultado] = useState('execucao')
  const [compilando, setCompilando] = useState(false)
  const [erro, setErro] = useState(null)
  const [erroDetalhes, setErroDetalhes] = useState([])
  const [showExemplos, setShowExemplos] = useState(false)
  const [editorWidth, setEditorWidth] = useState(58)
  const editorRef = useRef(null)
  const compileActionRef = useRef(() => {})
  const draggingSplitRef = useRef(false)
  const containerRef = useRef(null)

  const handleCompile = useCallback(async () => {
    if (!codigo.trim()) return

    setCompilando(true)
    setErro(null)
    setErroDetalhes([])

    try {
      const res = await compilar(codigo)
      if (res.sucesso) {
        setResultado(res)
        setAbaResultado('execucao')
        setErroDetalhes([])
      } else {
        setErro(res.erro || 'Erro desconhecido')
        setErroDetalhes(Array.isArray(res.erros) ? res.erros : [])
        setResultado(null)
      }
    } catch {
      setErro('Erro ao conectar com o servidor. Verifique se o backend está rodando (python app.py)')
      setErroDetalhes([])
      setResultado(null)
    } finally {
      setCompilando(false)
    }
  }, [codigo])

  useEffect(() => {
    compileActionRef.current = () => {
      if (!compilando) {
        handleCompile()
      }
    }
  }, [compilando, handleCompile])

  useEffect(() => {
    function onMouseMove(e) {
      if (!draggingSplitRef.current || !containerRef.current) return
      const rect = containerRef.current.getBoundingClientRect()
      if (rect.width <= 0) return
      const next = ((e.clientX - rect.left) / rect.width) * 100
      const clamped = Math.min(80, Math.max(20, next))
      setEditorWidth(clamped)
    }

    function onMouseUp() {
      draggingSplitRef.current = false
    }

    window.addEventListener('mousemove', onMouseMove)
    window.addEventListener('mouseup', onMouseUp)
    return () => {
      window.removeEventListener('mousemove', onMouseMove)
      window.removeEventListener('mouseup', onMouseUp)
    }
  }, [])

  const handleExemploClick = (exemplo) => {
    setCodigo(exemplo.codigo)
    setShowExemplos(false)
    setResultado(null)
    setErro(null)
    setErroDetalhes([])
  }

  const handleVisualGenerate = (generatedCode) => {
    setCodigo(generatedCode)
    setModo('editor')
    setResultado(null)
    setErro(null)
    setErroDetalhes([])
  }

  return (
    <div className="app">
      {/* Header */}
      <header className="header">
        <div className="header-left">
          <div className="logo">
            <div className="logo-icon">HS</div>
            <span className="logo-text">HomeScript IDE</span>
          </div>
        </div>

        <div className="header-center">
          <button className={`tab-btn ${modo === 'editor' ? 'active' : ''}`}
            onClick={() => setModo('editor')}>
            Editor
          </button>
          <button className={`tab-btn ${modo === 'visual' ? 'active' : ''}`}
            onClick={() => setModo('visual')}>
            Visual Builder
          </button>
        </div>

        <div className="header-right">
          <div className="status-indicator">
            <div className="status-dot"></div>
            <span>Pronto</span>
          </div>
          <div className="examples-dropdown">
            <button className="examples-btn" onClick={() => setShowExemplos(!showExemplos)}>
              Exemplos
            </button>
            {showExemplos && (
              <div className="examples-list">
                {EXEMPLOS.map((ex, i) => (
                  <button key={i} className="example-item" onClick={() => handleExemploClick(ex)}>
                    {ex.nome}
                  </button>
                ))}
              </div>
            )}
          </div>
        </div>
      </header>

              {modo === 'visual' ? (
        <div style={{ flex: 1, overflow: 'auto' }}>
          <VisualBuilder onGenerate={handleVisualGenerate} />
        </div>
      ) : (
        <>
          {/* Barra de compilação */}
              <div className="compile-bar">
            <button
              className={`compile-btn ${compilando ? 'loading' : ''}`}
              onClick={handleCompile}
              disabled={compilando || !codigo.trim()}
            >
              {compilando ? 'Compilando...' : 'Compilar (Ctrl/Cmd + Enter)'}
            </button>
          </div>

          {/* Área principal */}
          <div className="main-content" ref={containerRef}>
            {/* Editor */}
            <div className="editor-panel" style={{ flex: `0 0 ${editorWidth}%` }}>
              <div className="panel-header">
                <span className="panel-title">homescript (.iot)</span>
              </div>
              <div className="editor-wrapper">
                <Editor
                  height="100%"
                  language="homescript"
                  theme="homescript-ide"
                  value={codigo}
                  beforeMount={configurarHomeScript}
                  onMount={(editor, monaco) => {
                    editorRef.current = editor
                    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
                      compileActionRef.current()
                    })
                    editor.focus()
                  }}
                  onChange={(value) => setCodigo(value ?? '')}
                  options={{
                    automaticLayout: true,
                    fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
                    fontSize: 14,
                    fontLigatures: true,
                    lineHeight: 22,
                    lineNumbers: 'on',
                    minimap: { enabled: false },
                    glyphMargin: true,
                    folding: true,
                    scrollBeyondLastLine: false,
                    wordWrap: 'on',
                    tabSize: 4,
                    insertSpaces: true,
                    detectIndentation: false,
                    quickSuggestions: { other: true, comments: false, strings: false },
                    suggestOnTriggerCharacters: true,
                    snippetSuggestions: 'top',
                    renderLineHighlight: 'all',
                    scrollbar: {
                      verticalScrollbarSize: 10,
                      horizontalScrollbarSize: 10
                    }
                  }}
                />
              </div>
            </div>

            <div
              className="panel-resizer"
              onMouseDown={() => {
                draggingSplitRef.current = true
              }}
              role="separator"
              aria-orientation="vertical"
              aria-label="Redimensionar paineis"
            />

            {/* Painel de Resultado */}
            <div className="result-panel" style={{ flex: `0 0 ${100 - editorWidth}%` }}>
              <div className="result-tabs">
                <button className={`result-tab ${abaResultado === 'codigo' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('codigo')}>
                  Código C
                </button>
                <button className={`result-tab ${abaResultado === 'execucao' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('execucao')}>
                  Execução
                </button>
                <button className={`result-tab ${abaResultado === 'tokens' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('tokens')}>
                  Tokens
                </button>
                <button className={`result-tab ${abaResultado === 'ast' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('ast')}>
                  AST
                </button>
              </div>

              <div className="result-content">
                {erro ? (
                  <div className="error-box fade-in">
                    <div>❌ {erro}</div>
                    {erroDetalhes.length > 0 && (
                      <div className="error-list">
                        {erroDetalhes.map((item, idx) => (
                          <div key={idx} className="error-item">
                            {`[${item.fase || 'erro'}] linha ${item.linha ?? '-'}, coluna ${item.coluna ?? '-'}: ${item.mensagem || 'Erro sem detalhes'}`}
                          </div>
                        ))}
                      </div>
                    )}
                  </div>
                ) : (
                  <>
                    {abaResultado === 'tokens' && <TokensView tokens={resultado?.tokens} />}
                    {abaResultado === 'ast' && <ASTView ast={resultado?.ast} />}
                    {abaResultado === 'codigo' && <CodeView code={resultado?.codigo_c} />}
                    {abaResultado === 'execucao' && (
                      <ExecutionTraceView
                        key={`${codigo.length}-${resultado?.tokens?.length || 0}-${resultado?.codigo_c?.length || 0}`}
                        codigo={codigo}
                        resultado={resultado}
                      />
                    )}
                  </>
                )}
              </div>
            </div>
          </div>
        </>
      )}
    </div>
  )
}

export default App
