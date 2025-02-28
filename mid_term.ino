#include <WiFi.h>  // Use <ESP8266WiFi.h> if using ESP8266
#include <FirebaseESP32.h>  // Use <FirebaseESP8266.h> if using ESP8266

// WiFi Credentials
#define WIFI_SSID "Albert's Net"
#define WIFI_PASSWORD "uzumaki-naruto"

// Firebase Credentials
#define FIREBASE_HOST "https://pir-data-a8205-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "5b38ec3fb31400597285b66047df4894df1757e3"

#define PIR_SENSOR_1 13  // PIR Sensor 1 (Entrance)
#define PIR_SENSOR_2 14  // PIR Sensor 2 (Exit)

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int peopleCount = 0;
bool pir1_triggered = false;
bool pir2_triggered = false;
unsigned long timestamp1 = 0;
unsigned long timestamp2 = 0;

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi!");

    // Connect to Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    pinMode(PIR_SENSOR_1, INPUT);
    pinMode(PIR_SENSOR_2, INPUT);
}

void loop() {
    if (digitalRead(PIR_SENSOR_1) == HIGH && !pir1_triggered) {
        pir1_triggered = true;
        timestamp1 = millis();
        delay(50);
    }

    if (digitalRead(PIR_SENSOR_2) == HIGH && !pir2_triggered) {
        pir2_triggered = true;
        timestamp2 = millis();
        delay(50);
    }

    if (pir1_triggered && pir2_triggered) {
        if (timestamp1 < timestamp2) {
            peopleCount++;  // Someone entered
            Serial.print("Person Entered - Count: ");
        } else {
            peopleCount--;  // Someone left
            Serial.print("Person Left - Count: ");
        }
        Serial.println(peopleCount);

        // Send to Firebase
        if (Firebase.RTDB.setInt(&fbdo, "/peopleCount", peopleCount)) {
            Serial.println("Firebase update successful!");
        } else {
            Serial.print("Firebase update failed: ");
            Serial.println(fbdo.errorReason());
        }

        // Reset triggers
        pir1_triggered = false;
        pir2_triggered = false;
        timestamp1 = 0;
        timestamp2 = 0;

        delay(500);
    }
}
