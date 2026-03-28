"""
HomeScript - Ponte entre Python e o compilador C
Executa o compilador como subprocesso e captura a saída JSON.
"""

import subprocess
import json
import os
import tempfile

# Caminho do executável do compilador
COMPILER_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "homescript.exe")


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

        if resultado.returncode != 0:
            return {
                "sucesso": False,
                "erro": resultado.stderr.strip() if resultado.stderr else "Erro desconhecido na compilação",
                "tokens": [],
                "ast": None,
                "codigo_c": ""
            }

        # Parse da saída JSON
        try:
            dados = json.loads(resultado.stdout)
            # O compilador agora inclui 'sucesso' e 'erros' no JSON
            if "sucesso" not in dados:
                dados["sucesso"] = True  # compatibilidade
            if "erros" not in dados:
                dados["erros"] = []
            return dados
        except json.JSONDecodeError:
            return {
                "sucesso": False,
                "erro": f"Erro ao interpretar saída do compilador: {resultado.stdout[:200]}",
                "erros": [],
                "tokens": [],
                "ast": None,
                "codigo_c": ""
            }

    except subprocess.TimeoutExpired:
        return {
            "sucesso": False,
            "erro": "Tempo limite excedido (10 segundos)",
            "tokens": [],
            "ast": None,
            "codigo_c": ""
        }
    except FileNotFoundError:
        return {
            "sucesso": False,
            "erro": f"Compilador não encontrado em: {COMPILER_PATH}",
            "tokens": [],
            "ast": None,
            "codigo_c": ""
        }
    finally:
        # Remove arquivo temporário
        if os.path.exists(temp_path):
            os.unlink(temp_path)
