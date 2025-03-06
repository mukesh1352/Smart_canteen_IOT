```
#include <WiFi.h>
#include <FirebaseESP32.h>

// WiFi Credentials
#define WIFI_SSID "Albert's Net"
#define WIFI_PASSWORD "uzumaki-naruto"

// Firebase Credentials
#define FIREBASE_HOST "https://pir-data-a8205-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "5b38ec3fb31400597285b66047df4894df1757e3"

#define PIR_SENSOR_1 13  // PIR Sensor at Entrance
#define PIR_SENSOR_2 14  // PIR Sensor at Exit

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
    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
        retryCount++;
        if (retryCount > 20) {
            Serial.println("\nFailed to connect to WiFi. Restarting...");
            ESP.restart();
        }
    }
    Serial.println("\nConnected to WiFi!");

    // Connect to Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Set up PIR sensor pins
    pinMode(PIR_SENSOR_1, INPUT);
    pinMode(PIR_SENSOR_2, INPUT);
}

void loop() {
    // Read PIR sensors
    bool pir1_state = digitalRead(PIR_SENSOR_1);
    bool pir2_state = digitalRead(PIR_SENSOR_2);

    // Detect motion at Entrance
    if (pir1_state == HIGH && !pir1_triggered) {
        pir1_triggered = true;
        timestamp1 = millis();
        delay(200); // Debounce delay
    }

    // Detect motion at Exit
    if (pir2_state == HIGH && !pir2_triggered) {
        pir2_triggered = true;
        timestamp2 = millis();
        delay(200); // Debounce delay
    }

    // If both sensors triggered, determine if a person entered or exited
    if (pir1_triggered && pir2_triggered) {
        if (timestamp1 < timestamp2) {
            peopleCount++;  // Person entered
            Serial.println("Person Entered! ✅");
        } else {
            if (peopleCount > 0){
              peopleCount--;  // Person exited
              Serial.println("Person Exited! ❌");
            }
        }

        Serial.print("Capacity: ");
        Serial.println(peopleCount);

        // Send data to Firebase
        if (Firebase.setInt(fbdo, "/peopleCount", peopleCount)) {
            log_i("Firebase updated successfully!");
        } else {
            log_e("Firebase update failed: %s", fbdo.errorReason().c_str());
        }

        // Reset triggers
        pir1_triggered = false;
        pir2_triggered = false;
        timestamp1 = 0;
        timestamp2 = 0;

        delay(1500);
    }
}


```