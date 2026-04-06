"""
HomeScript - Ponte entre Python e o compilador C
Executa o compilador como subprocesso e captura a saída JSON.
"""

import subprocess
import json
import sys
import os
import tempfile

# Caminho do executável do compilador
exe_name = "homescript.exe" if sys.platform == "win32" else "homescript"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Suporta execução local em dois cenários:
# 1) executável na raiz do projeto
# 2) executável dentro de compiler/ (gerado por `make -C compiler`)
compiler_candidates = [
    os.path.join(PROJECT_ROOT, exe_name),
    os.path.join(PROJECT_ROOT, "compiler", exe_name),
]

COMPILER_PATH = next((path for path in compiler_candidates if os.path.exists(path)), compiler_candidates[0])


def compilar(codigo_homescript: str) -> dict:
    """
    Recebe código HomeScript como string, salva em arquivo temporário,
    executa o compilador e retorna o resultado como dicionário.
    """
    # Cria arquivo temporário com o código
    with tempfile.NamedTemporaryFile(mode="w", suffix=".iot", delete=False, encoding="utf-8") as f:
        f.write(codigo_homescript)
        temp_path = f.name

    try:
        # Executa o compilador com saída JSON
        resultado = subprocess.run(
            [COMPILER_PATH, temp_path, "--json"],
            capture_output=True,
            text=True,
            timeout=10
        )

        # Parse da saída JSON (mesmo em caso de código de saída != 0)
        try:
            dados = json.loads(resultado.stdout)
            if "sucesso" not in dados:
                dados["sucesso"] = (resultado.returncode == 0)
            dados.setdefault("erro", "")
            dados.setdefault("erros", [])
            dados.setdefault("tokens", [])
            dados.setdefault("ast", None)
            dados.setdefault("codigo_c", "")
            return dados
        except json.JSONDecodeError:
            if resultado.returncode != 0:
                erro_saida = resultado.stderr.strip() if resultado.stderr else "Erro desconhecido na compilação"
            else:
                erro_saida = f"Erro ao interpretar saída do compilador: {resultado.stdout[:200]}"
            return {
                "sucesso": False,
                "erro": erro_saida,
                "erros": [],
                "tokens": [],
                "ast": None,
                "codigo_c": ""
            }

    except subprocess.TimeoutExpired:
        return {
            "sucesso": False,
            "erro": "Tempo limite excedido (10 segundos)",
            "erros": [],
            "tokens": [],
            "ast": None,
            "codigo_c": ""
        }
    except FileNotFoundError:
        return {
            "sucesso": False,
            "erro": f"Compilador não encontrado em: {COMPILER_PATH}",
            "erros": [],
            "tokens": [],
            "ast": None,
            "codigo_c": ""
        }
    finally:
        # Remove arquivo temporário
        if os.path.exists(temp_path):
            os.unlink(temp_path)
