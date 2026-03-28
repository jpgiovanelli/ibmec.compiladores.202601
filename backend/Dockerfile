# Usa uma imagem leve do Python
FROM python:3.12-slim

# Instala o GCC para podermos compilar o compilador HomeScript
RUN apt-get update && apt-get install -y gcc make && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copia os diretórios do compilador e exemplos (necessários para compilar e para a API)
COPY lexer/ lexer/
COPY parser/ parser/
COPY codegen/ codegen/
COPY compiler/ compiler/
COPY exemplos/ exemplos/

# Compila o compilador (gera o executável "homescript" no Linux)
RUN gcc -Wall -Wextra -std=c99 -o homescript compiler/main.c lexer/lexer.c parser/parser.c parser/ast.c codegen/codegen.c

# Copia e configura o backend
COPY backend/ backend/

WORKDIR /app/backend

# Instala as dependências do FastAPI/Uvicorn
RUN pip install --no-cache-dir -r requirements.txt

EXPOSE 8000

# Executa o servidor na porta 8000
CMD ["python", "app.py"]
