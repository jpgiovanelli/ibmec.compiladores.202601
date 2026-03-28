# 📘 Definição da Linguagem HomeScript

## Visão Geral

A **HomeScript** é uma linguagem de programação de alto nível projetada para automação residencial. Ela permite que o usuário declare dispositivos, sensores e regras de comportamento de forma simples e legível, sem precisar escrever código em C ou lidar com registradores de hardware.

Arquivos HomeScript utilizam a extensão **`.iot`**.

---

## Estrutura da Linguagem

### 1. Declaração de Dispositivos (`device`)

Declara um dispositivo controlável (ex: lâmpada, ventilador, motor).

```
device luz pin 13;
device ventilador pin 7;
device portao pin 3;
```

**Sintaxe:** `device <nome> pin <número>;`

---

### 2. Declaração de Sensores (`sensor`)

Declara um sensor de entrada (ex: movimento, temperatura, luminosidade).

```
sensor movimento pin 2;
sensor temperatura pin A0;
sensor luminosidade pin A1;
```

**Sintaxe:** `sensor <nome> pin <pino>;`

---

### 3. Comandos de Ação (`turn on` / `turn off`)

Liga ou desliga um dispositivo.

```
turn luz on;
turn ventilador off;
```

**Sintaxe:** `turn <dispositivo> on;` ou `turn <dispositivo> off;`

---

### 4. Comando de Espera (`wait`)

Pausa a execução por N milissegundos.

```
wait 1000;
wait 5000;
```

**Sintaxe:** `wait <milissegundos>;`

---

### 5. Estrutura Condicional (`if`)

Executa um bloco se a condição for verdadeira.

```
if temperatura > 30 {
    turn ventilador on;
}
```

**Sintaxe:** `if <sensor> <operador> <valor> { ... }`

---

### 6. Estrutura de Evento (`when`)

Define uma regra que é avaliada continuamente (como um gatilho).

```
when movimento == detected {
    turn luz on;
    wait 5000;
    turn luz off;
}
```

**Sintaxe:** `when <sensor> <operador> <valor> { ... }`

---

### 7. Blocos `{ }`

Todos os blocos de código (`if`, `when`) são delimitados por chaves `{ }`.

---

## Operadores Suportados

| Operador | Significado     |
|----------|-----------------|
| `==`     | Igual a         |
| `!=`     | Diferente de    |
| `>`      | Maior que       |
| `<`      | Menor que       |
| `>=`     | Maior ou igual  |
| `<=`     | Menor ou igual  |

---

## Valores Especiais

| Valor       | Significado                    |
|-------------|--------------------------------|
| `detected`  | Sensor detectou algo (HIGH)    |
| `not_detected` | Sensor não detectou (LOW)   |
| `on`        | Estado ligado                  |
| `off`       | Estado desligado               |

---

## Exemplos de Código `.iot`

### Exemplo 1 — Luz com sensor de movimento

```
device luz pin 13;
sensor movimento pin 2;

when movimento == detected {
    turn luz on;
    wait 5000;
    turn luz off;
}
```

**Descrição:** Quando o sensor de movimento detecta presença, liga a luz por 5 segundos e depois desliga.

---

### Exemplo 2 — Ventilador com sensor de temperatura

```
device ventilador pin 7;
sensor temperatura pin A0;

if temperatura > 30 {
    turn ventilador on;
}

if temperatura < 25 {
    turn ventilador off;
}
```

**Descrição:** Liga o ventilador quando a temperatura passa de 30°C e desliga quando cai abaixo de 25°C.

---

### Exemplo 3 — Sequência com delay

```
device led_vermelho pin 10;
device led_verde pin 11;
device led_azul pin 12;

turn led_vermelho on;
wait 1000;
turn led_vermelho off;

turn led_verde on;
wait 1000;
turn led_verde off;

turn led_azul on;
wait 1000;
turn led_azul off;
```

**Descrição:** Acende LEDs em sequência (vermelho → verde → azul), cada um por 1 segundo.
