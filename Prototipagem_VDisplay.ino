#include "HX711.h"  // Biblioteca ESP32-HX711
#include <Wire.h>   // Biblioteca para comunicação I2C
#include <LiquidCrystal_I2C.h>

HX711 balanca;                       // Instância da balança HX711
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Endereço do display I2C

int DOUT_PIN = 18;  // Escolha o pino GPIO adequado para DOUT (por exemplo, GPIO 18)
int CLK_PIN = 19;   // Escolha o pino GPIO adequado para CLK (por exemplo, GPIO 19)

float calibration_factor = 11777896;  // Fator de calibração para teste inicial

void setup() {
  balanca.begin(DOUT_PIN, CLK_PIN);  // Inicializa o HX711 com os pinos definidos
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);  // Ajuste o tamanho do LCD conforme suas configurações

  Serial.begin(9600);

  // Inicializa a balança
  Serial.println();
  Serial.println("HX711 - Calibracao da Balanca");
  Serial.println("Remova o peso da balanca");
  Serial.println("Depois que as leituras começarem, coloque um peso conhecido sobre a Balanca");
  Serial.println("Pressione a, s, d, f para aumentar o Fator de Calibracao por 10, 100, 1000, 10000 respectivamente");
  Serial.println("Pressione z, x, c, v para diminuir o Fator de Calibracao por 10, 100, 1000, 10000 respectivamente");
  Serial.println("Após leitura correta do peso, pressione t para TARA (zerar)");

  balanca.set_scale();  // Configura a escala da balanca
  zeraBalanca();        // Zera a Balanca
}

void zeraBalanca() {
  Serial.println();
  balanca.tare();
  Serial.println("Balanca Zerada");
}

void loop() {
  balanca.set_scale(calibration_factor);

  float pesoGramas = balanca.get_units() * 26.57;  // Peso em gramas
  Serial.print("Peso: ");
  Serial.print(pesoGramas, 3);
  Serial.println(" g");

  // Verifique se o peso atingiu 100g
  if (pesoGramas >= 0.100) {
    lcd.setCursor(2, 0);
    lcd.print("Porcao de 100g atingida!");

    // Você pode adicionar aqui o código para ativar um dispositivo ou executar uma ação quando 100g forem atingidos.
  }

  delay(500);

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

  if (pesoGramas < 0.100) {
    lcd.setCursor(2, 0);
    lcd.print("SMART FEEDER");
    lcd.setCursor(2, 1);
    lcd.print("Peso: ");
    lcd.print(pesoGramas, 3);
    lcd.println("g   ");
    // Você pode adicionar aqui o código para ativar um dispositivo ou executar uma ação quando 100g forem atingidos.
  }


  delay(1000);
}
