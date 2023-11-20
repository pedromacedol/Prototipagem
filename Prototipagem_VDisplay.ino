#include "HX711.h"  // Biblioteca ESP32-HX711
#include <Wire.h>   // Biblioteca para comunicação I2C
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

HX711 balanca;                       // Instância da balança HX711
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Endereço do display I2C

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
uint8_t colPins[COLS] = { 25, 33, 32, 15 };
uint8_t rowPins[ROWS] = { 14, 12, 26, 27 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int DOUT_PIN = 18;
int CLK_PIN = 19;

float calibration_factor = 11777896;  // Fator de calibração para teste inicial
float pesoDaPorcao = 0.100;           // Peso de uma porção em gramas
int qtdPorcoesDesejadas = 0;          // Quantidade de porções desejadas
float pesoDesejado = 0.0;             // Peso total desejado

bool pesoAtingido = false;  // Indicador para verificar se o peso desejado foi atingido

bool tecladoAtivado = false;  // Indicador para verificar se o teclado foi ativado

void zeraBalanca() {
  Serial.println();
  balanca.tare();
  Serial.println("Balanca Zerada");
}

void setup() {
  balanca.begin(DOUT_PIN, CLK_PIN);
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Serial.begin(9600);

  Serial.println();
  Serial.println("HX711 - Calibracao da Balanca");
  Serial.println("Remova o peso da balanca");
  Serial.println("Digite a quantidade de porções desejadas e pressione # para confirmar");

  balanca.set_scale(calibration_factor);
  zeraBalanca();
}

void loop() {
  if (!tecladoAtivado) {
    char key = keypad.getKey();
    if (key != NO_KEY && isDigit(key)) {
      qtdPorcoesDesejadas = qtdPorcoesDesejadas * 10 + (key - '0');
      Serial.print("Quantidade de Porções Desejadas: ");
      Serial.println(qtdPorcoesDesejadas);
    } else if (key == '#') {
      pesoDesejado = qtdPorcoesDesejadas * pesoDaPorcao;
      tecladoAtivado = true;  // Ativar o teclado apenas uma vez
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Peso Desejado:");
      lcd.setCursor(0, 1);
      lcd.print(pesoDesejado, 2);
      lcd.print(" g");
      delay(2000);
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Aguarde...");
      delay(1000);
    }
  } else {
    float pesoGramas = balanca.get_units() * 26.57;  // Leitura do peso em gramas (média de 10 leituras)

    Serial.print("Peso: ");
    Serial.print(pesoGramas, 3);
    Serial.println(" g");

    if (pesoGramas < pesoDaPorcao) {
      char key = keypad.getKey();
      if (key != NO_KEY && isDigit(key)) {
        qtdPorcoesDesejadas = qtdPorcoesDesejadas * 10 + (key - '0');
        Serial.print("Quantidade de Porções Desejadas: ");
        Serial.println(qtdPorcoesDesejadas);
      } else if (key == '#') {
        pesoDesejado = qtdPorcoesDesejadas * pesoDaPorcao;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Peso Desejado:");
        lcd.setCursor(0, 1);
        lcd.print(pesoDesejado, 2);
        lcd.print(" g");
        delay(2000);
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Aguarde...");
        delay(1000);
      }
      if (pesoDesejado == 0) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("SMART FEEDER");
    } else if (pesoGramas != 0) {
      if (!pesoAtingido) {
        if (pesoGramas >= pesoDesejado) {
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("Peso atingido!");
          pesoAtingido = true;
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Peso Atual:");
          lcd.setCursor(0, 1);
          lcd.print(pesoGramas, 3);
          lcd.print(" g");
        }
      } 
    }
      
    }

    if (pesoDesejado > 0 && pesoGramas >= pesoDesejado) {
      dispenseFood();
    }

    if (Serial.available()) {
      char temp = Serial.read();
      if (temp == '+' || temp == 'a')
        calibration_factor += 10;
      else if (temp == '-' || temp == 'z')
        calibration_factor -= 10;
      else if (temp == 's')
        calibration_factor += 100;
      else if (temp == 'x')
        calibration_factor -= 100;
      else if (temp == 'd')
        calibration_factor += 1000;
      else if (temp == 'c')
        calibration_factor -= 1000;
      else if (temp == 'f')
        calibration_factor += 10000;
      else if (temp == 'v')
        calibration_factor -= 10000;
      else if (temp == 't')
        zeraBalanca();
    }

    delay(1000);  // Atraso opcional, ajuste conforme necessário
  }
}

void dispenseFood() {
  qtdPorcoesDesejadas = 0;
  pesoDesejado = 0;
  pesoAtingido = false;
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Comida dispensada");
  lcd.setCursor(1, 1);
  lcd.print("Peso atingido!");
  delay(3000);
}
