/*
 * HTTP Client Manager - Handles HTTP POST to Google Apps Script
 */

#ifndef HTTP_CLIENT_MANAGER_H
#define HTTP_CLIENT_MANAGER_H

#include <HTTPClient.h>
#include <Preferences.h>
#include "config.h"

class HTTPClientManager {
private:
  String serverUrl;
  
public:
  HTTPClientManager() {}
  
  void begin(Preferences& preferences) {
    serverUrl = preferences.getString("serverUrl", "");
    
    if (serverUrl.length() == 0) {
      Serial.println("Warning: No server URL configured");
    } else {
      Serial.println("Server URL: " + serverUrl);
    }
  }
  
  int sendData(String jsonPayload) {
    if (serverUrl.length() == 0) {
      Serial.println("Error: No server URL configured");
      return -1;
    }
    
    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT);
    
    Serial.println("Connecting to: " + serverUrl);
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error code: " + String(httpResponseCode));
      Serial.println("Error: " + http.errorToString(httpResponseCode));
    }
    
    http.end();
    return httpResponseCode;
  }
};

#endif
