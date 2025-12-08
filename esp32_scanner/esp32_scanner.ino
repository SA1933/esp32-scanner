/*
 * ESP32 Barcode Scanner
 * 
 * Scan barcode → Enter quantity → Send to Google Sheets via HTTP POST
 * Supports Wi-Fi config (AP/STA), LCD 20x4 feedback, multiple devices
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi_manager.h"
#include "scanner_manager.h"
#include "display_manager.h"
#include "http_client_manager.h"

// Global objects
Preferences preferences;
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
WiFiManager wifiManager;
ScannerManager scannerManager;
DisplayManager displayManager(lcd);
HTTPClientManager httpManager;

// Device ID (unique per device)
String deviceId;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 Barcode Scanner Starting...");
  
  // Initialize preferences
  preferences.begin("scanner", false);
  
  // Get or create device ID
  deviceId = preferences.getString("deviceId", "");
  if (deviceId.length() == 0) {
    deviceId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    preferences.putString("deviceId", deviceId);
  }
  Serial.println("Device ID: " + deviceId);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  displayManager.showStartup(deviceId);
  
  // Initialize WiFi
  displayManager.showStatus("WiFi Setup...");
  if (!wifiManager.begin(preferences, displayManager)) {
    Serial.println("WiFi setup failed!");
    displayManager.showError("WiFi Failed!");
    delay(2000);
  }
  
  // Initialize scanner
  scannerManager.begin();
  
  // Initialize HTTP manager
  httpManager.begin(preferences);
  
  displayManager.showReady();
  Serial.println("System Ready");
}

void loop() {
  // Check WiFi connection
  if (!wifiManager.isConnected()) {
    displayManager.showStatus("WiFi Lost...");
    wifiManager.reconnect();
    if (wifiManager.isConnected()) {
      displayManager.showReady();
    }
  }
  
  // Check for barcode scan
  if (scannerManager.available()) {
    String barcode = scannerManager.read();
    
    if (barcode.length() > 0) {
      Serial.println("Scanned: " + barcode);
      handleBarcodeScan(barcode);
    }
  }
  
  delay(100);
}

void handleBarcodeScan(String barcode) {
  // Display scanned barcode
  displayManager.showBarcode(barcode);
  delay(1000);
  
  // Get quantity from user
  int quantity = getQuantityInput();
  
  if (quantity > 0) {
    // Send data to server
    sendDataToServer(barcode, quantity);
  } else {
    displayManager.showError("Cancelled");
    delay(1500);
    displayManager.showReady();
  }
}

int getQuantityInput() {
  displayManager.showQuantityPrompt();
  
  int quantity = 0;
  unsigned long startTime = millis();
  String input = "";
  
  // Wait for quantity input via serial (or buttons if configured)
  while (millis() - startTime < QUANTITY_TIMEOUT) {
    if (Serial.available()) {
      char c = Serial.read();
      
      if (c == '\n' || c == '\r') {
        if (input.length() > 0) {
          quantity = input.toInt();
          break;
        }
      } else if (c >= '0' && c <= '9') {
        input += c;
        displayManager.showQuantityInput(input);
      } else if (c == 27) { // ESC key
        return 0;
      }
    }
    delay(50);
  }
  
  if (quantity == 0 && input.length() == 0) {
    // Timeout or cancelled
    return 0;
  }
  
  return quantity;
}

void sendDataToServer(String barcode, int quantity) {
  displayManager.showStatus("Sending...");
  
  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["deviceId"] = deviceId;
  doc["barcode"] = barcode;
  doc["quantity"] = quantity;
  doc["timestamp"] = getTimestamp();
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  Serial.println("Sending: " + jsonPayload);
  
  // Send with retries
  int retries = 0;
  bool success = false;
  
  while (retries < MAX_RETRIES && !success) {
    if (retries > 0) {
      Serial.println("Retry " + String(retries) + "/" + String(MAX_RETRIES));
      displayManager.showStatus("Retry " + String(retries) + "...");
      delay(RETRY_DELAY);
    }
    
    int statusCode = httpManager.sendData(jsonPayload);
    
    if (statusCode == 200 || statusCode == 201) {
      success = true;
      displayManager.showSuccess("Sent OK!");
      Serial.println("Data sent successfully!");
    } else {
      retries++;
      Serial.println("Failed with code: " + String(statusCode));
    }
  }
  
  if (!success) {
    displayManager.showError("Send Failed!");
    Serial.println("Failed after " + String(MAX_RETRIES) + " retries");
  }
  
  delay(2000);
  displayManager.showReady();
}

String getTimestamp() {
  // Simple timestamp - milliseconds since boot
  // In production, use NTP for real time
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  char timestamp[32];
  sprintf(timestamp, "%02lu:%02lu:%02lu", hours % 24, minutes % 60, seconds % 60);
  return String(timestamp);
}
