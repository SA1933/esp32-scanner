#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

Preferences prefs;
WebServer server(80);
bool apMode = false;

HardwareSerial GM65(1);

#define R1 KEYPAD_R1
#define R2 KEYPAD_R2
#define R3 KEYPAD_R3
#define R4 KEYPAD_R4
#define C1 KEYPAD_C1
#define C2 KEYPAD_C2
#define C3 KEYPAD_C3
#define C4 KEYPAD_C4

String sku = "";
String input = "";
bool waitingQty = false;
bool negative = false;

String itemNameShort = "";
String itemNameShort2 = "";
String itemNameShort3 = "";
long currentStock = 0;

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
void fetchItemInfo(const String &skuCode);

void setup() {
  Serial.begin(115200);

  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  
  setupKeypad();
  GM65.begin(SCANNER_BAUD, SERIAL_8N1, SCANNER_RX, SCANNER_TX);

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

      fetchItemInfo(sku);
      displaySKULocked();
    }
  }

  if (waitingQty) {
    char key = readKeypad();
    if (key != 0) handleKey(key);
  }
  delay(50);
}
