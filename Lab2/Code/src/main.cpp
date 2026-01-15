#include <Arduino.h>

const int sensorPin = A0; 

void setup() {

  Serial.begin(115200);

  pinMode(sensorPin, INPUT);
  delay(1000);
  Serial.println("Starting Voltage Measurement...");
}

void loop() {

  int adcValue = analogRead(sensorPin);
  
  float voltage = (adcValue / 4095.0) * 3.3;

  Serial.print("Raw ADC: ");
  Serial.print(adcValue);
  Serial.print("\t Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");

  delay(500);
}