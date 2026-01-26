#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
// Client Code
#include "BLEDevice.h"
//#include "BLEScan.h"

// TODO: change the service UUID to the one you are using on the server side.
// The remote service we wish to connect to.
static BLEUUID serviceUUID("620a250f-a7fd-477e-b7ac-9441f02e3f51");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("1baf8efc-fa6a-451b-ae5b-53ca7c988cbd");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// TODO: define new global variables for data collection
// current distance received
static float g_currentDist = 0.0;
// minimum distance since connection
static float g_minDist = 0.0;
// maximum distance since connection
static float g_maxDist = 0.0;
// flag to indicate if at least one data has been received
static bool g_hasData = false;

// TODO: define a new function for data aggregation


void aggregateDistanceData(uint8_t* pData, size_t length) {
  // change the string to float number and update min/max values
  String msg = "";
  for (size_t i = 0; i < length; i++) {
    msg += (char)pData[i];
  }
  float dist = msg.toFloat();

  // save the current distance
  g_currentDist = dist;

  // 3. update min and max
  if (!g_hasData) {
    // first time receiving data, initialize min and max with current value
    g_minDist = dist;
    g_maxDist = dist;
    g_hasData = true;
  } else {
    if (dist < g_minDist) {
      g_minDist = dist;
    }
    if (dist > g_maxDist) {
      g_maxDist = dist;
    }
  }
}

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

    // update the distance data
    aggregateDistanceData(pData, length);

    // print current, min, max distance
    Serial.println("===== New distance data received =====");
    Serial.print("Current distance: ");
    Serial.println(g_currentDist);

    Serial.print("Min distance since connection: ");
    Serial.println(g_minDist);

    Serial.print("Max distance since connection: ");
    Serial.println(g_maxDist);
    Serial.println("======================================");
}


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    // only show our own device
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(serviceUUID)) {

      Serial.println(">>> FOUND SERVER <<<");
      Serial.print("Address: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

// This is the Arduino main loop function.
void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
      g_hasData = false;  
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
 
  if (connected && g_hasData) {
    Serial.print("[Summary] Current: ");
    Serial.print(g_currentDist);
    Serial.print("  Min: ");
    Serial.print(g_minDist);
    Serial.print("  Max: ");
    Serial.println(g_maxDist);
  } else if (doScan) {
    BLEDevice::getScan()->start(0);
  }

  delay(1000);
}
