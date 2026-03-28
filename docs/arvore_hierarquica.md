# 🌳 Árvore Hierárquica — HomeScript

## Código de Entrada

```
when movimento == detected {
    turn luz on;
}
```

## Árvore Hierárquica (formato textual indentado)

```
Program
└── WhenStatement
    ├── Condição
    │   ├── Sensor: movimento
    │   ├── Operador: ==
    │   └── Valor: detected
    └── Bloco
        └── TurnCommand
            ├── Dispositivo: luz
            └── Estado: on
```

---

## Exemplo Completo

### Código

```
device luz pin 13;
sensor movimento pin 2;

when movimento == detected {
    turn luz on;
    wait 5000;
    turn luz off;
}
```

### Árvore

```
Program
├── DeviceDeclaration
│   ├── Nome: luz
│   └── Pino: 13
├── SensorDeclaration
│   ├── Nome: movimento
│   └── Pino: 2
└── WhenStatement
    ├── Condição
    │   ├── Sensor: movimento
    │   ├── Operador: ==
    │   └── Valor: detected
    └── Bloco
        ├── TurnCommand
        │   ├── Dispositivo: luz
        │   └── Estado: on
        ├── WaitCommand
        │   └── Tempo: 5000
        └── TurnCommand
            ├── Dispositivo: luz
            └── Estado: off
```

---

## Exemplo com `if`

### Código

```
device ventilador pin 7;
sensor temperatura pin A0;

if temperatura > 30 {
    turn ventilador on;
}
```

### Árvore

```
Program
├── DeviceDeclaration
│   ├── Nome: ventilador
│   └── Pino: 7
├── SensorDeclaration
│   ├── Nome: temperatura
│   └── Pino: A0
└── IfStatement
    ├── Condição
    │   ├── Sensor: temperatura
    │   ├── Operador: >
    │   └── Valor: 30
    └── Bloco
        └── TurnCommand
            ├── Dispositivo: ventilador
            └── Estado: on
```
