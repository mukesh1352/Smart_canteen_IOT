#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi Credentials
#define WIFI_SSID "Albert's Net"
#define WIFI_PASSWORD "uzumaki-naruto"

// Firebase Credentials
#define FIREBASE_HOST "water-depth-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "dc460d2b348d74bc1f6d389a012f2f960f345fe3"

// Water Sensor Pin
#define WATER_SENSOR_PIN 34  

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Time sync using NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST timezone (GMT+5:30)

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
    Serial.println("\n✅ Connected to WiFi!");

    // Start NTP Client
    timeClient.begin();
    timeClient.forceUpdate();

    // Configure Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // *Clear Firebase on fresh run*
    if (!firebaseCleared) {
        if (Firebase.deleteNode(fbdo, "/waterLog")) {
            Serial.println("🗑 Cleared Firebase data.");
        } else {
            Serial.println("⚠ Failed to clear Firebase: " + fbdo.errorReason());
        }
        firebaseCleared = true;
    }

    // *Test Firebase Write*
    if (Firebase.setInt(fbdo, "/test", 123)) {
        Serial.println("✅ Test Firebase write successful!");
    } else {
        Serial.println("❌ Test Firebase write failed: " + fbdo.errorReason());
    }

    // Set up water sensor pin
    pinMode(WATER_SENSOR_PIN, INPUT);
}

// Function to get current formatted time
String getFormattedTime() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);

    char buffer[30];
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
        ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return String(buffer);
}

void loop() {
    int waterLevel = analogRead(WATER_SENSOR_PIN);
    delay(100); // Allow sensor to stabilize

    // *Handle sensor failure case*
    if (waterLevel == 0) {
        Serial.println("⚠ No water level detected! Check sensor connections.");
        return;
    }

    // Get current time
    String currentTime = getFormattedTime();
    unsigned long logID = millis();  // Unique ID for each entry

    Serial.print("💧 Water Level: ");
    Serial.println(waterLevel);
    Serial.print("⏰ Time: ");
    Serial.println(currentTime);

    // *Store water level data in a structured Firebase format*
    String path = "/waterLog/" + String(logID);
    
    if (Firebase.setInt(fbdo, path + "/level", waterLevel) &&
        Firebase.setString(fbdo, path + "/time", currentTime)) {
        Serial.println("✅ Firebase updated successfully!");
    } else {
        Serial.println("❌ Firebase update failed: " + fbdo.errorReason());
    }

    delay(5000); // Wait before next reading
}