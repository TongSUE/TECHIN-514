/*  hello_xiao    A first sketch for the Seeed Studio XIAO ESP32C3 in PlatformIO*
  
    *This example code is in the public domain.*
  
    *Michel Deslierres  June 15, 2020 */
  
  #include <Arduino.h>  // needed in PlatformIO*
  
  void setup() {
    Serial.begin(115200);
  }
  
  void loop() {
    Serial.println("Hello XIAO!");
    delay(2000);
  }