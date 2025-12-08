/*
 * Display Manager - Handles LCD 20x4 display output
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <LiquidCrystal_I2C.h>
#include "config.h"

class DisplayManager {
private:
  LiquidCrystal_I2C& lcd;
  
  void clear() {
    lcd.clear();
  }
  
  void printCentered(int row, String text) {
    int len = text.length();
    int start = (LCD_COLS - len) / 2;
    if (start < 0) start = 0;
    
    lcd.setCursor(start, row);
    lcd.print(text.substring(0, LCD_COLS));
  }
  
public:
  DisplayManager(LiquidCrystal_I2C& lcdDisplay) : lcd(lcdDisplay) {}
  
  void showStartup(String deviceId) {
    clear();
    printCentered(0, "ESP32 Scanner");
    printCentered(1, "v1.0");
    lcd.setCursor(0, 3);
    lcd.print(deviceId.substring(0, LCD_COLS));
    delay(2000);
  }
  
  void showStatus(String message) {
    clear();
    printCentered(1, message);
  }
  
  void showError(String error) {
    clear();
    printCentered(1, "ERROR:");
    printCentered(2, error);
  }
  
  void showSuccess(String message) {
    clear();
    printCentered(1, "SUCCESS:");
    printCentered(2, message);
  }
  
  void showReady() {
    clear();
    printCentered(1, "Ready to Scan");
    printCentered(2, "Scan Barcode...");
  }
  
  void showBarcode(String barcode) {
    clear();
    lcd.setCursor(0, 0);
    lcd.print("Barcode:");
    lcd.setCursor(0, 1);
    lcd.print(barcode.substring(0, LCD_COLS));
  }
  
  void showQuantityPrompt() {
    lcd.setCursor(0, 2);
    lcd.print("Enter Quantity:");
    lcd.setCursor(0, 3);
    lcd.print("Qty: ");
  }
  
  void showQuantityInput(String input) {
    lcd.setCursor(5, 3);
    lcd.print("               "); // Clear rest of line
    lcd.setCursor(5, 3);
    lcd.print(input);
  }
  
  void showWiFiConnected(String ip) {
    clear();
    printCentered(0, "WiFi Connected!");
    lcd.setCursor(0, 2);
    lcd.print("IP: ");
    lcd.print(ip.substring(0, 16));
  }
  
  void showAPMode(String ip) {
    clear();
    printCentered(0, "Config Mode");
    lcd.setCursor(0, 1);
    lcd.print("SSID: ");
    lcd.print(AP_SSID);
    lcd.setCursor(0, 2);
    lcd.print("Pass: ");
    lcd.print(AP_PASSWORD);
    lcd.setCursor(0, 3);
    lcd.print("IP: ");
    lcd.print(ip);
  }
};

#endif
