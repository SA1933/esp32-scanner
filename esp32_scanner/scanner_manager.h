/*
 * Scanner Manager - Handles barcode scanner input
 */

#ifndef SCANNER_MANAGER_H
#define SCANNER_MANAGER_H

#include <HardwareSerial.h>
#include "config.h"

class ScannerManager {
private:
  HardwareSerial scannerSerial;
  String buffer;
  
public:
  ScannerManager() : scannerSerial(2) {}  // Use Serial2
  
  void begin() {
    scannerSerial.begin(SCANNER_BAUD, SERIAL_8N1, SCANNER_RX_PIN, SCANNER_TX_PIN);
    Serial.println("Scanner initialized on Serial2");
    buffer = "";
  }
  
  bool available() {
    while (scannerSerial.available()) {
      char c = scannerSerial.read();
      
      // Most barcode scanners send CR or LF at the end
      if (c == '\r' || c == '\n') {
        if (buffer.length() > 0) {
          return true;
        }
      } else {
        buffer += c;
      }
    }
    return false;
  }
  
  String read() {
    String result = buffer;
    buffer = "";
    return result;
  }
};

#endif
