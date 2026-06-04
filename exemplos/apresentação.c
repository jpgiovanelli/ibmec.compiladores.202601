#include <Arduino.h>
#include <DHT.h>

#define luz_sala 13
#define luz_corredor 12
#define luz_jardim 11
#define ventilador_quarto 10
#define aquecedor_quarto 9
#define sirene 8
#define bomba_caixa 7
#define portao_motor 6
#define fita_led_garagem 5

int movimento_sala_pin = 2;
int porta_entrada_pin = 3;

// Potenciômetros
int luminosidade_jardim_pin = A0; // potenciômetro simulando luminosidade
int nivel_caixa_pin = A1;         // potenciômetro simulando nível da caixa

DHT temperatura_quarto_dht(4, DHT11);

// Se estiver usando Arduino Uno, cuidado: 14 = A0 e 15 = A1.
// Melhor usar A2 e A3 para o sensor ultrassônico.
int distancia_garagem_trig_pin = A2;
int distancia_garagem_echo_pin = A3;

int limite_luz_jardim = 500;
int temperatura_max = 30;
int temperatura_min = 22;
int nivel_minimo_caixa = 380;
int distancia_abertura_portao = 10;

int contador_alertas = 0;
int contador_movimento = 0;
int i = 0;
int tentativas_portao = 0;

long read_distancia_garagem_cm() {
    digitalWrite(distancia_garagem_trig_pin, LOW);
    delayMicroseconds(2);

    digitalWrite(distancia_garagem_trig_pin, HIGH);
    delayMicroseconds(10);

    digitalWrite(distancia_garagem_trig_pin, LOW);

    long duration = pulseIn(distancia_garagem_echo_pin, HIGH, 30000);
    return duration / 58;
}

void setup() {
    pinMode(luz_sala, OUTPUT);
    pinMode(luz_corredor, OUTPUT);
    pinMode(luz_jardim, OUTPUT);
    pinMode(ventilador_quarto, OUTPUT);
    pinMode(aquecedor_quarto, OUTPUT);
    pinMode(sirene, OUTPUT);
    pinMode(bomba_caixa, OUTPUT);
    pinMode(portao_motor, OUTPUT);
    pinMode(fita_led_garagem, OUTPUT);

    pinMode(movimento_sala_pin, INPUT);
    pinMode(porta_entrada_pin, INPUT);

    pinMode(luminosidade_jardim_pin, INPUT);
    pinMode(nivel_caixa_pin, INPUT);

    temperatura_quarto_dht.begin();

    pinMode(distancia_garagem_trig_pin, OUTPUT);
    pinMode(distancia_garagem_echo_pin, INPUT);
    digitalWrite(distancia_garagem_trig_pin, LOW);

    Serial.begin(9600);
}

void loop() {
    // Sensor de movimento
    if (digitalRead(movimento_sala_pin) == LOW) {
        contador_movimento = contador_movimento + 1;
        digitalWrite(luz_sala, HIGH);
    }
    else {
        digitalWrite(luz_sala, LOW);
    }

    // Sensor da porta
    if (digitalRead(porta_entrada_pin) == LOW) {
        contador_alertas = contador_alertas + 1;
        digitalWrite(luz_corredor, HIGH);

        for (i = 0; i < 3; i = i + 1) {
            digitalWrite(sirene, HIGH);
            delay(250);
            digitalWrite(sirene, LOW);
            delay(250);
        }
    }
    else {
        digitalWrite(luz_corredor, LOW);
    }

    // Potenciômetro simulando luminosidade do jardim
    int leitura_luminosidade = analogRead(luminosidade_jardim_pin);

    if (leitura_luminosidade < limite_luz_jardim) {
        digitalWrite(luz_jardim, HIGH);
    }
    else {
        digitalWrite(luz_jardim, LOW);
    }

    // Sensor de temperatura DHT22
    float temperatura = temperatura_quarto_dht.readTemperature();

    if (!isnan(temperatura)) {
        if (temperatura > temperatura_max) {
            digitalWrite(ventilador_quarto, HIGH);
            digitalWrite(aquecedor_quarto, LOW);
        }
        else {
            if (temperatura < temperatura_min) {
                digitalWrite(aquecedor_quarto, HIGH);
                digitalWrite(ventilador_quarto, LOW);
            }
            else {
                digitalWrite(ventilador_quarto, LOW);
                digitalWrite(aquecedor_quarto, LOW);
            }
        }
    }

    // Potenciômetro simulando nível da caixa
    int leitura_nivel = analogRead(nivel_caixa_pin);

    if (leitura_nivel < nivel_minimo_caixa) {
        digitalWrite(bomba_caixa, HIGH);
    }
    else {
        digitalWrite(bomba_caixa, LOW);
    }

    // Sensor ultrassônico da garagem
    long distancia_garagem = read_distancia_garagem_cm();

    if (distancia_garagem < distancia_abertura_portao) {
        tentativas_portao = 0;

        while (tentativas_portao < 2) {
            digitalWrite(portao_motor, HIGH);
            digitalWrite(fita_led_garagem, HIGH);
            delay(1200);

            digitalWrite(portao_motor, LOW);
            delay(300);

            tentativas_portao = tentativas_portao + 1;
        }
    }
    else {
        digitalWrite(portao_motor, LOW);
        digitalWrite(fita_led_garagem, LOW);
    }

    // Monitor Serial
    Serial.print("Movimentos: ");
    Serial.println(contador_movimento);

    Serial.print("Alertas: ");
    Serial.println(contador_alertas);

    Serial.print("Luminosidade simulada: ");
    Serial.println(leitura_luminosidade);

    Serial.print("Nivel da caixa simulado: ");
    Serial.println(leitura_nivel);

    Serial.print("Temperatura: ");
    Serial.println(temperatura);

    Serial.print("Distancia garagem: ");
    Serial.println(distancia_garagem);

    Serial.println("----------------------");

    delay(500);
}