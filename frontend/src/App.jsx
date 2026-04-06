import { useState, useCallback } from 'react'
import './index.css'

/* ===== Exemplos de código HomeScript ===== */
const EXEMPLOS = [
  {
    nome: '💡 Luz com Sensor',
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
    nome: '🌡️ Ventilador com Temperatura',
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
    nome: '🏠 Automação Completa',
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

async function compilar(codigo) {
  const res = await fetch(`${API_URL}/compile`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ codigo })
  })
  return res.json()
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
  const [abaResultado, setAbaResultado] = useState('codigo')
  const [compilando, setCompilando] = useState(false)
  const [erro, setErro] = useState(null)
  const [erroDetalhes, setErroDetalhes] = useState([])
  const [showExemplos, setShowExemplos] = useState(false)

  const handleCompile = useCallback(async () => {
    if (!codigo.trim()) return

    setCompilando(true)
    setErro(null)
    setErroDetalhes([])

    try {
      const res = await compilar(codigo)
      if (res.sucesso) {
        setResultado(res)
        setAbaResultado('codigo')
        setErroDetalhes([])
      } else {
        setErro(res.erro || 'Erro desconhecido')
        setErroDetalhes(Array.isArray(res.erros) ? res.erros : [])
        setResultado(null)
      }
    } catch (e) {
      setErro('Erro ao conectar com o servidor. Verifique se o backend está rodando (python app.py)')
      setErroDetalhes([])
      setResultado(null)
    } finally {
      setCompilando(false)
    }
  }, [codigo])

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
            <div className="logo-icon">🏠</div>
            <span className="logo-text">HomeScript IDE</span>
          </div>
        </div>

        <div className="header-center">
          <button className={`tab-btn ${modo === 'editor' ? 'active' : ''}`}
            onClick={() => setModo('editor')}>
            ✏️ Editor
          </button>
          <button className={`tab-btn ${modo === 'visual' ? 'active' : ''}`}
            onClick={() => setModo('visual')}>
            🧩 Visual
          </button>
        </div>

        <div className="header-right">
          <div className="status-indicator">
            <div className="status-dot"></div>
            <span>Pronto</span>
          </div>
          <div className="examples-dropdown">
            <button className="examples-btn" onClick={() => setShowExemplos(!showExemplos)}>
              📂 Exemplos
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
              {compilando ? '⏳ Compilando...' : '▶ Compilar'}
            </button>
          </div>

          {/* Área principal */}
          <div className="main-content">
            {/* Editor */}
            <div className="editor-panel">
              <div className="panel-header">
                <span className="panel-title">📝 código homescript (.iot)</span>
              </div>
              <div className="editor-wrapper">
                <textarea
                  className="code-editor"
                  value={codigo}
                  onChange={e => setCodigo(e.target.value)}
                  placeholder="// Escreva seu código HomeScript aqui..."
                  spellCheck={false}
                />
              </div>
            </div>

            {/* Painel de Resultado */}
            <div className="result-panel">
              <div className="result-tabs">
                <button className={`result-tab ${abaResultado === 'codigo' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('codigo')}>
                  ⚡ Código C
                </button>
                <button className={`result-tab ${abaResultado === 'tokens' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('tokens')}>
                  🔤 Tokens
                </button>
                <button className={`result-tab ${abaResultado === 'ast' ? 'active' : ''}`}
                  onClick={() => setAbaResultado('ast')}>
                  🌳 AST
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
