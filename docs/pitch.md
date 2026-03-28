# 🎤 Pitch Técnico-Comercial — HomeScript

---

## 🔴 Slide 1 — O Problema

**Automação residencial é complicada demais.**

Hoje, para programar uma casa inteligente, o usuário precisa:

- Conhecer linguagens complexas como C/C++ ou Python
- Entender registradores, pinos digitais e protocolos de comunicação
- Configurar IDEs, compiladores e bibliotecas
- Debugar código de baixo nível em microcontroladores

**Resultado:** Automação residencial é restrita a engenheiros e programadores experientes. Pessoas comuns ficam dependentes de soluções caras e proprietárias.

---

## 🟢 Slide 2 — A Solução

**HomeScript: programe sua casa como se estivesse conversando com ela.**

Uma linguagem de programação simples e intuitiva que qualquer pessoa pode aprender em minutos:

```
device luz pin 13;
sensor movimento pin 2;

when movimento == detected {
    turn luz on;
    wait 5000;
    turn luz off;
}
```

O compilador HomeScript converte esse código em C/Arduino automaticamente — pronto para rodar em hardware real.

---

## 🔵 Slide 3 — O Diferencial

| Característica | Mercado Atual | HomeScript |
|---|---|---|
| Linguagem | C/C++ complexo | Simples e legível |
| Público | Engenheiros | Qualquer pessoa |
| Idioma | Inglês | Português e Inglês |
| Custo | Plataformas pagas | Open source |
| Interface | Terminal/IDE | Web visual + código |
| Resultado | Código fonte | C compilável gerado |

**Diferenciais-chave:**
- 🌍 **Multi-idioma:** `turn on` ou `ligar` — funciona dos dois jeitos
- 🖥️ **Interface visual:** Monte automações arrastando blocos, sem escrever código
- ⚡ **Compilação real:** Gera código C funcional para Arduino/ESP32

---

## 🟡 Slide 4 — Exemplo na Prática

### O que o usuário escreve (HomeScript):
```
device ventilador pin 7;
sensor temperatura pin A0;

if temperatura > 30 {
    turn ventilador on;
}
```

### O que o sistema gera (C/Arduino):
```c
#include <Arduino.h>

#define ventilador 7
int temperatura_pin = A0;

void setup() {
    pinMode(ventilador, OUTPUT);
    pinMode(temperatura_pin, INPUT);
}

void loop() {
    int temperatura = analogRead(temperatura_pin);
    if (temperatura > 30) {
        digitalWrite(ventilador, HIGH);
    }
}
```

**De 3 linhas legíveis para código completo funcional — automaticamente.**
