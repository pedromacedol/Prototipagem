#include "HX711.h" 
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "thingProperties.h"
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

float calibrationFactor = -11777896;  // Calibration factor of balance
float portionWeight = 0.100;         // Portion weigth = 0.100kg
bool isFull = false;
float weight = 0;
float measuredWeight = 0;
char key;

int DOUT_PIN = 18;
int CLK_PIN = 19;

void resetDigitalScale() {
  digitaScale.tare();
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  initProperties();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);

  digitaScale.begin(DOUT_PIN, CLK_PIN);
  digitaScale.set_scale(calibrationFactor);
  resetDigitalScale();

  myServo.attach(servoPin);
  myServo.write(0);
}

void loop() {
  measuredWeight = digitaScale.get_units() * 26.57;
  if (isKey) {
  lcd.setCursor(2, 0);
  lcd.print("SMART FEEDER");
  lcd.setCursor(0, 1);
  lcd.print("Quantidade:");
  key = keypad.getKey();

  if (key != NO_KEY) {
    onPortionsChange();
     if (key == '#'){
      onButtonChange();
     }
  }

  } else {
    Serial.print("Peso: ");
    Serial.print(measuredWeight, 3);
    Serial.println(" g");
    displayMeasuredWeight = measuredWeight;
    
    if (measuredWeight >= weight) {
      fullBowl();
    }

    if (Serial.available()) {
      char temp = Serial.read();
      balanceCalibration(temp);
    }

    delay(1000);

  }
  ArduinoCloud.update();
}




void fullBowl() {
  portions = 0;
  weight = 0;
  displayMeasuredWeight = 0;
  isFull = false;
  isKey = true;
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Peso atingido,");
  lcd.setCursor(0, 1);
  lcd.print("remova a racao");
  delay(100);
  lcd.clear();
  myServo.write(0);
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

/*
  Since Portions is READ_WRITE variable, onPortionsChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onPortionsChange()  {
  lcd.clear();
  lcd.setCursor(12, 1);
  if (isDigit(key)) {
    portions = portions * 10 + (key - '0');
  }
  lcd.print(portions);
  Serial.print("Quantidade de Porções Desejadas: ");
  Serial.println(portions);
}
/*
  Since Button is READ_WRITE variable, onButtonChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onButtonChange()  {
  resetDigitalScale();
  weight = (portions * portionWeight) ;
  isKey = false;
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
/*
  Since SendPortions is READ_WRITE variable, onSendPortionsChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onSendPortionsChange()  {
  onButtonChange();
}


/*
  Since MeasuredWeight is READ_WRITE variable, onMeasuredWeightChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMeasuredWeightChange()  {
  // Add your code here to act upon MeasuredWeight change
}
