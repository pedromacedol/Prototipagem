#include "HX711.h" 
#include <Wire.h>   
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>

Servo myServo;
int servoPin = 23; 

HX711 digitaScale;                  
LiquidCrystal_I2C lcd(0x3F, 16, 2); 

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

float calibrationFactor = -11777896;  // Calibration factor of balance
float portionWeight = 0.100;         // Portion weigth = 0.100kg
int portions = 0;
float weight = 0.0;
bool isFull = false;
bool isKey = false;  

void resetDigitalScale() {
  Serial.println();
  digitaScale.tare();
  Serial.println("Balança Zerada");
}



void setup() {
  digitaScale.begin(DOUT_PIN, CLK_PIN);
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Serial.begin(9600);

  Serial.println();
  Serial.println("HX711 - Calibracao da digitaScale");
  Serial.println("Remova o peso da balanca");
  Serial.println("Digite a quantidade de porções desejadas e pressione # para confirmar");

  digitaScale.set_scale(calibrationFactor);
  resetDigitalScale();

  myServo.attach(servoPin);
  myServo.write(0);
}

void loop() {
  float measuredWeight = digitaScale.get_units() * 26.57;
  if (!isKey) {
    lcd.setCursor(2, 0);
    lcd.print("SMART FEEDER");
    lcd.setCursor(0, 1);
    lcd.print("Quantidade:");
    char key = keypad.getKey();

    if (key != NO_KEY) {
      selectPortions(key);
    }

  } else {
    Serial.print("Peso: ");
    Serial.print(measuredWeight, 3);
    Serial.println(" g");

    if (measuredWeight >= weight) {
      fullBowl();
    }

    if (Serial.available()) {
      char temp = Serial.read();
      balanceCalibration(temp);
    }

    delay(1000);
  }
}

void fullBowl() {
  portions = 0;
  weight = 0;
  isFull = false;
  isKey = false;
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Peso atingido,");
  lcd.setCursor(0, 1);
  lcd.print("remova a racao");
  delay(100);
  lcd.clear();
  myServo.write(0);
}

void selectPortions(char key) {
  if (isDigit(key)) {
    lcd.setCursor(12, 1);
    portions = portions * 10 + (key - '0');
    lcd.print(portions);
    Serial.print("Quantidade de Porções Desejadas: ");
    Serial.println(portions);
  } else if (key == '#') {
    resetDigitalScale();
    weight = (portions * portionWeight) ;
    isKey = true;
    myServo.write(45);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Peso Desejado:");
    lcd.setCursor(0, 1);
    lcd.print(weight, 2);
    lcd.print(" g");
    delay(2000);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Aguarde...");
    delay(1000);
  }
}

void balanceCalibration(char temp) {
  if (temp == '+' || temp == 'a')
    calibrationFactor += 10;
  else if (temp == '-' || temp == 'z')
    calibrationFactor -= 10;
  else if (temp == 's')
    calibrationFactor += 100;
  else if (temp == 'x')
    calibrationFactor -= 100;
  else if (temp == 'd')
    calibrationFactor += 1000;
  else if (temp == 'c')
    calibrationFactor -= 1000;
  else if (temp == 'f')
    calibrationFactor += 10000;
  else if (temp == 'v')
    calibrationFactor -= 10000;
  else if (temp == 't')
    resetDigitalScale();
}
