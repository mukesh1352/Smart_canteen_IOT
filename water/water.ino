#include <WiFi.h>
#include <FirebaseESP32.h>

// WiFi Credentials
#define WIFI_SSID "Deepak"
#define WIFI_PASSWORD "qwerty123"

// Firebase Credentials
#define FIREBASE_HOST "water-depth-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "dc460d2b348d74bc1f6d389a012f2f960f345fe3"

// Water Sensor Pin
#define WATER_SENSOR_PIN 34  

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Flag to track if Firebase has been cleared
bool firebaseCleared = false;

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\n Connected to WiFi!");

    // Configure Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Clear Firebase on fresh run
    if (!firebaseCleared) {
        if (Firebase.deleteNode(fbdo, "/waterLog")) {
            Serial.println("Cleared Firebase data.");
        } else {
            Serial.println(" Failed to clear Firebase: " + fbdo.errorReason());
        }
        firebaseCleared = true;
    }

    // Set up water sensor pin
    pinMode(WATER_SENSOR_PIN, INPUT);
}

// Function to log water level
void logWaterLevel(int waterLevel) {
    unsigned long timestamp = millis(); // Timestamp in milliseconds

    Serial.print("Water Level: ");
    Serial.println(waterLevel);
    Serial.print("Timestamp: ");
    Serial.println(timestamp);

    // Store data in Firebase
    String path = "/waterLog/" + String(timestamp);

    FirebaseJson json;
    json.add("timestamp", timestamp);
    json.add("level", waterLevel);

    if (Firebase.setJSON(fbdo, path, json)) {
        Serial.println("Firebase updated successfully!");
    } else {
        Serial.println("Firebase update failed: " + fbdo.errorReason());
    }
}

void loop() {
    int waterLevel = analogRead(WATER_SENSOR_PIN);
    delay(100); // Allow sensor to stabilize

    // If no water is detected, log "0"
    if (waterLevel == 0) {
        Serial.println("No water level detected! Logging 0 to Firebase.");
        logWaterLevel(0);
    } else {
        logWaterLevel(waterLevel);
    }

    delay(5000); // Wait before next reading
}