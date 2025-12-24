#include <Arduino.h>
#include "config.h"

#ifndef R1
#define R1 KEYPAD_R1
#define R2 KEYPAD_R2
#define R3 KEYPAD_R3
#define R4 KEYPAD_R4
#define C1 KEYPAD_C1
#define C2 KEYPAD_C2
#define C3 KEYPAD_C3
#define C4 KEYPAD_C4
#endif

extern String sku;
extern String input;
extern bool waitingQty;
extern bool negative;
extern bool apMode;
void displayReset();
void updateQtyDisplay();
void displaySending();
void sendData(String sku, String qty);
void startAP();
void resetWiFiSettings();

void setupKeypad() {
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);
  pinMode(C1, INPUT_PULLUP);
  pinMode(C2, INPUT_PULLUP);
  pinMode(C3, INPUT_PULLUP);
  pinMode(C4, INPUT_PULLUP);
  
  digitalWrite(R1, HIGH);
  digitalWrite(R2, HIGH);
  digitalWrite(R3, HIGH);
  digitalWrite(R4, HIGH);
}

char readKeypad() {
  static unsigned long lastTime = 0;
  static char lastKey = 0;
  static char holdKey = 0;
  static unsigned long holdStart = 0;
  
  if (millis() - lastTime < KEYPAD_DEBOUNCE) return 0;
  lastTime = millis();
  
  char keys[4][4] = {
    {'1', '4', '7', '*'},
    {'2', '5', '8', '0'},
    {'3', '6', '9', '#'},
    {'A', 'B', 'C', 'D'}
  };
  
  int rows[4] = {R1, R2, R3, R4};
  int cols[4] = {C1, C2, C3, C4};
  
  for (int row = 0; row < 4; row++) {
    for (int r = 0; r < 4; r++) {
      digitalWrite(rows[r], (r == row) ? LOW : HIGH);
    }
    delayMicroseconds(10);
    
    for (int col = 0; col < 4; col++) {
      int state = digitalRead(cols[col]);
      if (state == LOW) {
        char key = keys[row][col];

        if (key == '*') {
          if (holdKey == 0) {
            holdKey = '*';
            holdStart = millis();
          } else if (holdKey == '*' && (millis() - holdStart >= 3000)) {

            lastKey = 0;
            holdKey = 0;
            return 'L';
          }
        }
        
        if (key != lastKey) {
          lastKey = key;
          return key;
        }
        return 0;
      }
    }
  }

  if (holdKey != 0 && (millis() - holdStart < 3000)) {

    holdKey = 0;
  }
  lastKey = 0;
  return 0;
}

void handleKey(char key) {

  extern void startAP();
  extern void resetWiFiSettings();
  
  switch (key) {
    case 'A':
      if (input.length() > 0) input.remove(input.length() - 1);
      else if (negative) negative = false;
      updateQtyDisplay();
      break;
      
    case 'B':
      negative = !negative;
      updateQtyDisplay();
      break;
      
    case 'C':
      sku = "";
      waitingQty = false;
      input = "";
      negative = false;
      displayReset();
      Serial.println("\nReset - Scan SKU");
      break;

    case '#':
      sku = "";
      waitingQty = false;
      input = "";
      negative = false;
      displayReset();
      Serial.println("\nBack - Ready to Scan");
      break;
    
    case 'L':
      
      resetWiFiSettings();
      break;
      
    case 'D':
      if (input.length() > 0) {
        int qty = input.toInt();
        if (negative) qty = -qty;

        sendData(sku, String(qty));
      }
      break;
      
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      input += key;
      updateQtyDisplay();
      break;
  }
}
