#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// WiFi credentials
#define WIFI_SSID "UW MPSK"
#define WIFI_PASSWORD "xxxx"

// Firebase credentials
#define WEB_API_KEY "xxxx"
#define USER_EMAIL "xxxx"
#define USER_PASSWORD "xxxx"
#define DB_URL "xxxx"

// Duration 14 seconds
const unsigned long STAGE_INTERVAL = 14000;

// Ultrasonic pins
const int trigPin = D0;
const int echoPin = D1;
const float soundSpeed = 0.034;

// Firebase related objects
UserAuth user_auth(WEB_API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;

WiFiClientSecure ssl_client1, ssl_client2;
using AsyncClient = AsyncClientClass;
AsyncClient async_client1(ssl_client1), async_client2(ssl_client2);

RealtimeDatabase Database;
AsyncResult dbResult;     // Used to receive async results

// Firebase
unsigned long sendDataPrevMillis = 0;
int uploadInterval = 4000;   // Upload once every 4 seconds

// ultrasonic
unsigned long readingLoopPrevMillis = 0;
const unsigned long readingLoopInterval = 1000;  // Run uploadReadingLoop every 1 second

// Function declarations
float measureDistance();
void connectToWiFi();
void initFirebase();
void sendDataToFirebase(float distance);
void processData(AsyncResult &aResult);
void uploadReadingLoop();
void asyncCB(AsyncResult &aResult) {
  processData(aResult);
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  unsigned long startTime = millis();

  connectToWiFi();
  delay(1000); // wait a bit after WiFi connection

  initFirebase();

  // Wait for Firebase app to be ready
  Serial.println("Waiting for Firebase app to be ready...");
  unsigned long t0 = millis();
  while (!app.ready() && millis() - t0 < 10000) {
    app.loop();
  }
  if (!app.ready()) {
    Serial.println("Firebase app not ready, skip sending.");
  } else {
    Serial.println("Firebase app is ready. Start uploading readings.");
  }

  startTime = millis();
  while (millis() - startTime < STAGE_INTERVAL) {
    app.loop();

    uploadReadingLoop();

    delay(50);
  }

  // Deep sleep
  Serial.println("Go to deep sleep for 30 seconds...");
  WiFi.disconnect(true);
  delay(100);
  esp_sleep_enable_timer_wakeup(30 * 1000000);
  esp_deep_sleep_start();
}

void loop() {}

// Measure distance
float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * soundSpeed / 2.0;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}

// Connect WiFi
void connectToWiFi() {
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry++;
    if (retry > 20) {
      Serial.println("\nWiFi connect failed, restarting...");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Initialize Firebase
void initFirebase() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  ssl_client1.setInsecure();
  ssl_client2.setInsecure();

  ssl_client1.setHandshakeTimeout(5);
  ssl_client2.setHandshakeTimeout(5);

  Serial.println("Initializing Firebase app...");
  initializeApp(async_client1, app, getAuth(user_auth), processData, "authTask");

  app.getApp<RealtimeDatabase>(Database);

  Database.url(DB_URL);

  Serial.print("DB_URL in code: ");
  Serial.println(DB_URL);

  async_client1.setAsyncResult(dbResult);
}

// Send distance data to Firebase
void sendDataToFirebase(float distance) {
  if (millis() - sendDataPrevMillis > (unsigned long)uploadInterval || sendDataPrevMillis == 0) {
    sendDataPrevMillis = millis();

    Serial.print("Pushing distance value... ");

    // Write to RTDB path /examples/BareMinimum/data/distance
    Database.set<float>(async_client1,
                        "/examples/BareMinimum/data/distance",
                        distance,
                        dbResult);
    if (async_client1.lastError().code() == 0) {
      Serial.println("ok");
    } else {
      Firebase.printf("Error, msg: %s, code: %d\n",
                      async_client1.lastError().message().c_str(),
                      async_client1.lastError().code());
    }
  }

  processData(dbResult);
}

// Upload
void uploadReadingLoop() {

  if (!app.ready()) {
    return;
  }

  if (millis() - readingLoopPrevMillis >= readingLoopInterval) {
    readingLoopPrevMillis = millis();

    float d = measureDistance();
    sendDataToFirebase(d);
  }
}

void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;

  if (aResult.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(),
                    aResult.error().message().c_str(),
                    aResult.error().code());
  }

  if (aResult.available()) {
    Firebase.printf("task: %s, payload: %s\n",
                    aResult.uid().c_str(),
                    aResult.c_str());
  }
}



