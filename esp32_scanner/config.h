/*
 * Configuration file for ESP32 Barcode Scanner
 */

#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define LCD_I2C_ADDR 0x27  // LCD I2C address (change if different)
#define LCD_COLS 20        // LCD columns
#define LCD_ROWS 4         // LCD rows

#define SCANNER_RX_PIN 16  // Serial RX pin for barcode scanner
#define SCANNER_TX_PIN 17  // Serial TX pin for barcode scanner
#define SCANNER_BAUD 9600  // Barcode scanner baud rate

// WiFi Configuration
#define AP_SSID "ESP32-Scanner-Config"
#define AP_PASSWORD "12345678"
#define WIFI_CONNECT_TIMEOUT 20000  // 20 seconds
#define WIFI_RECONNECT_DELAY 5000   // 5 seconds

// HTTP Configuration
#define MAX_RETRIES 3
#define RETRY_DELAY 2000  // 2 seconds
#define HTTP_TIMEOUT 10000  // 10 seconds

// Timing Configuration
#define QUANTITY_TIMEOUT 30000  // 30 seconds for quantity input

#endif
