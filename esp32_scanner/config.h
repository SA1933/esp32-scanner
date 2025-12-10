#ifndef CONFIG_H
#define CONFIG_H

const char* AP_SSID = "ESP-Scanner";
const char* AP_PASSWORD = "";

#define LCD_SDA 21
#define LCD_SCL 22
#define LCD_ADDRESS 0x27

#define SCANNER_RX 16
#define SCANNER_TX 17
#define SCANNER_BAUD 9600

#define KEYPAD_R1 13
#define KEYPAD_R2 12
#define KEYPAD_R3 14
#define KEYPAD_R4 27
#define KEYPAD_C1 26
#define KEYPAD_C2 25
#define KEYPAD_C3 33
#define KEYPAD_C4 32

#define WIFI_TIMEOUT 6000  
#define KEYPAD_DEBOUNCE 200
#define DISPLAY_DELAY 1000 

const char* GAS_URL = "--APP SCRIPT--";

const char* API_KEY = "";

#define LCD_COLS 20
#define LCD_ROWS 4

#endif
