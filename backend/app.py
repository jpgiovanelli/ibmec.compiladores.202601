"""
HomeScript - Backend API
Servidor FastAPI que expõe o compilador HomeScript via endpoints REST.

Endpoints:
  POST /compile       - Compila código HomeScript (retorna tokens + AST + código C)
  POST /tokens        - Retorna apenas os tokens (análise léxica)
  GET  /health        - Verifica se o servidor está funcionando
  GET  /exemplos      - Retorna exemplos de código HomeScript
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import Optional
import os

from compiler_bridge import compilar

# --- App FastAPI ---
app = FastAPI(
    title="HomeScript API",
    description="API do compilador HomeScript — transforma código .iot em C/Arduino",
    version="1.0.0"
)

# CORS (permite frontend acessar)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# --- Modelos ---
class CodigoEntrada(BaseModel):
    codigo: str

    class Config:
        json_schema_extra = {
            "example": {
                "codigo": 'device luz pin 13;\nsensor movimento pin 2;\n\nwhen movimento == detected {\n    turn luz on;\n    wait 5000;\n    turn luz off;\n}'
            }
        }

class ResultadoCompilacao(BaseModel):
    sucesso: bool
    tokens: Optional[list] = []
    ast: Optional[dict] = None
    codigo_c: Optional[str] = ""
    erro: Optional[str] = None
    erros: Optional[list] = []

# --- Endpoints ---

@app.get("/health")
async def health():
    """Verifica se o servidor está funcionando."""
    return {"status": "ok", "compilador": "HomeScript v1.0"}


@app.post("/compile", response_model=ResultadoCompilacao)
async def compile(entrada: CodigoEntrada):
    """
    Compila código HomeScript completo.
    Retorna tokens, AST e código C gerado.
    """
    if not entrada.codigo.strip():
        raise HTTPException(status_code=400, detail="Código vazio")

    resultado = compilar(entrada.codigo)
    return resultado


@app.post("/tokens")
async def tokens(entrada: CodigoEntrada):
    """
    Retorna apenas os tokens (análise léxica).
    """
    if not entrada.codigo.strip():
        raise HTTPException(status_code=400, detail="Código vazio")

    resultado = compilar(entrada.codigo)

    if not resultado.get("sucesso"):
        return {"sucesso": False, "erro": resultado.get("erro"), "tokens": []}

    return {"sucesso": True, "tokens": resultado.get("tokens", [])}


@app.get("/exemplos")
async def exemplos():
    """Retorna exemplos de código HomeScript."""
    exemplos_dir = os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "exemplos"
    )

    lista = []
    if os.path.exists(exemplos_dir):
        for arquivo in sorted(os.listdir(exemplos_dir)):
            if arquivo.endswith(".iot"):
                caminho = os.path.join(exemplos_dir, arquivo)
                with open(caminho, "r", encoding="utf-8") as f:
                    conteudo = f.read()
                lista.append({
                    "nome": arquivo,
                    "codigo": conteudo
                })

    return {"exemplos": lista}


# --- Executar ---
if __name__ == "__main__":
    import uvicorn
    uvicorn.run("app:app", host="0.0.0.0", port=8000, reload=True)
