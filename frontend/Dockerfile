# --- Estágio 1: Build da aplicação React/Vite ---
FROM node:20-alpine AS builder

WORKDIR /app

# Instala apenas as dependências primeiro (melhora o cache do Docker)
COPY frontend/package*.json ./
RUN npm install

# Copia o resto dos arquivos e roda o build
COPY frontend/ ./
RUN npm run build

# --- Estágio 2: Servidor NGINX estático ---
FROM nginx:alpine

# Copia o build final para o diretório de servidor do Nginx
COPY --from=builder /app/dist /usr/share/nginx/html

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
