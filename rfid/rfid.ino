#include <SPI.h>                // SPI library for RFID communication
#include <MFRC522.h>            // MFRC522 library for RFID
#include <Wire.h>               // Wire library for I2C communication
#include <LiquidCrystal_I2C.h>  // LiquidCrystal_I2C library for LCD

// RFID pins for ESP32
#define SS_PIN 5   // Define Slave Select pin for RFID
#define RST_PIN 2  // Define Reset pin for RFID

MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance with defined SS and RST pins

// I2C LCD address (replace 0x27 if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // 16x2 LCD with I2C address 0x27

void setup() {
  Serial.begin(115200);  // Start serial communication (ESP32 works better at 115200 baud)
  
  // Initialize SPI
  SPI.begin();  

  // Initialize RFID
  rfid.PCD_Init();  

  // Initialize LCD
  lcd.init();    
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID Card");
}

void loop() {
  // Check if a new RFID card is present
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    
    // Read UID and display on Serial Monitor
    Serial.print("UID: ");
    String content = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
      content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(rfid.uid.uidByte[i], HEX));
    }
    Serial.println();

    // Display UID on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card UID:");
    lcd.setCursor(0, 1);
    lcd.print(content);

    delay(1000);  // Show UID for 1 second before scanning the next card

    // Prepare for next scan
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(5000);
    
    // Reset LCD message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID Card");
  }
}


