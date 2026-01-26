#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;

// TODO: add new global variables for your sensor readings and processed data
const int trig = D0;
const int echo = D1;

int duration;         
float distance;   
float meter;

const int numReadings = 5;
int readings[numReadings];
int readIndex = 0;             
long total = 0;
int averageDistance = 0;
int lastValidDistance = 0;



#define SERVICE_UUID        "620a250f-a7fd-477e-b7ac-9441f02e3f51"
#define CHARACTERISTIC_UUID "1baf8efc-fa6a-451b-ae5b-53ca7c988cbd"

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};


void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);

    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0;
    }


    BLEDevice::init("FGONB");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Sever Name: FGONB");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
    digitalWrite(trig, HIGH);     
    delayMicroseconds(10);
    digitalWrite(trig, LOW);     
    duration = pulseIn(echo, HIGH); 

    
    int rawDistance = 0;
    if (duration > 0 && duration < 38000) {
        rawDistance = duration / 58;
        lastValidDistance = rawDistance;
    } else {
        rawDistance = lastValidDistance;
    }

    // DSP Moving Average
    total = total - readings[readIndex];
    readings[readIndex] = rawDistance;
    total = total + readings[readIndex];
    readIndex = readIndex + 1;

    if (readIndex >= numReadings) {
        readIndex = 0;
    }

    averageDistance = total / numReadings;

    Serial.print("Raw:");
    Serial.print(rawDistance);
    Serial.print("cm \t"); 
    Serial.print("Denoised:");
    Serial.print(averageDistance);
    Serial.println("cm");


    if (deviceConnected) {
        unsigned long currentMillis = millis();
        
        if (currentMillis - previousMillis >= interval) {
            
            previousMillis = currentMillis;

            if (averageDistance > 0 && averageDistance < 30) {
                String sendValue = String(averageDistance);
                pCharacteristic->setValue(sendValue.c_str());
                pCharacteristic->notify();
                
                Serial.print("Data Sent: ");
                Serial.println(averageDistance);
            } 
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}
