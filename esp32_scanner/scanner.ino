#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences prefs;
WebServer server(80);
bool apMode = false;

#define RX_PIN 16
#define TX_PIN 17
HardwareSerial GM65(1);

#define R1 13
#define R2 12
#define R3 14
#define R4 27
#define C1 26
#define C2 25
#define C3 33
#define C4 32

String sku = "";
String input = "";
bool waitingQty = false;
bool negative = false;

void connectWiFi(String ssid, String pass);
void startAP();
void setupKeypad();
char readKeypad();
void handleKey(char key);
void displaySKULocked();
void updateQtyDisplay();
void displayReset();
void displaySending();
void displaySent();
void sendData(String sku, String qty);
void showMainMenu();

void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1=Scan  2=New WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Press key to select");

  unsigned long start = millis();
  const unsigned long TIMEOUT = 10000;

  while (millis() - start < TIMEOUT) {
    server.handleClient();
    char k = readKeypad();
    if (k == '1') {
      displayReset();
      return;
    }
    if (k == '2') {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Entering AP mode");
      lcd.setCursor(0,1);
      lcd.print("Clearing WiFi creds");
      prefs.begin("wifi", false);
      prefs.clear();
      prefs.end();
      delay(500);
      startAP();
      return;
    }
    delay(50);
  }

  displayReset();
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  
  setupKeypad();
  GM65.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  prefs.end();
  
  if (ssid.length() > 0) {
    connectWiFi(ssid, pass);
  } else {
    startAP();
  }
}

void loop() {
  server.handleClient();
  if (apMode) return;
  
  if (GM65.available() && !waitingQty) {
    sku = GM65.readStringUntil('\n');
    sku.trim();
    if (sku.length() > 0) {
      waitingQty = true;
      input = "";
      negative = false;
      displaySKULocked();
    }
  }
  
  if (waitingQty) {
    char key = readKeypad();
    if (key != 0) handleKey(key);
  }
  delay(50);
}
