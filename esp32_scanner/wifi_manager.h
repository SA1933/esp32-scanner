/*
 * WiFi Manager - Handles WiFi connection in AP and STA modes
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include "display_manager.h"

class WiFiManager {
private:
  Preferences* prefs;
  DisplayManager* display;
  WebServer* server;
  String ssid;
  String password;
  bool apMode;
  
  void startAP();
  void handleRoot();
  void handleSave();
  void setupWebServer();
  
public:
  WiFiManager();
  ~WiFiManager();
  
  bool begin(Preferences& preferences, DisplayManager& displayManager);
  bool isConnected();
  void reconnect();
  String getIP();
};

WiFiManager::WiFiManager() {
  prefs = nullptr;
  display = nullptr;
  server = nullptr;
  apMode = false;
}

WiFiManager::~WiFiManager() {
  if (server != nullptr) {
    delete server;
  }
}

bool WiFiManager::begin(Preferences& preferences, DisplayManager& displayManager) {
  prefs = &preferences;
  display = &displayManager;
  
  // Try to load saved credentials
  ssid = prefs->getString("ssid", "");
  password = prefs->getString("password", "");
  
  if (ssid.length() > 0) {
    // Try to connect with saved credentials
    Serial.println("Connecting to: " + ssid);
    display->showStatus("Connecting...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECT_TIMEOUT) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected! IP: " + WiFi.localIP().toString());
      display->showWiFiConnected(WiFi.localIP().toString());
      delay(2000);
      return true;
    } else {
      Serial.println("Connection failed, starting AP mode");
    }
  }
  
  // Start AP mode for configuration
  startAP();
  return false;
}

void WiFiManager::startAP() {
  Serial.println("Starting AP mode...");
  apMode = true;
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP: " + IP.toString());
  
  display->showAPMode(IP.toString());
  
  setupWebServer();
}

void WiFiManager::setupWebServer() {
  if (server != nullptr) {
    delete server;
  }
  
  server = new WebServer(80);
  
  server->on("/", [this]() { handleRoot(); });
  server->on("/save", HTTP_POST, [this]() { handleSave(); });
  
  server->begin();
  Serial.println("Web server started");
}

void WiFiManager::handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 Scanner Config</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;margin:20px;}input{width:100%;padding:8px;margin:8px 0;}</style></head>";
  html += "<body><h1>ESP32 Scanner Configuration</h1>";
  html += "<form action='/save' method='POST'>";
  html += "WiFi SSID:<br><input name='ssid' required><br>";
  html += "Password:<br><input name='password' type='password' required><br>";
  html += "Server URL:<br><input name='serverUrl' placeholder='https://script.google.com/...' required><br>";
  html += "<input type='submit' value='Save and Connect'>";
  html += "</form></body></html>";
  
  server->send(200, "text/html", html);
}

void WiFiManager::handleSave() {
  String newSsid = server->arg("ssid");
  String newPassword = server->arg("password");
  String serverUrl = server->arg("serverUrl");
  
  if (newSsid.length() > 0) {
    prefs->putString("ssid", newSsid);
    prefs->putString("password", newPassword);
    prefs->putString("serverUrl", serverUrl);
    
    server->send(200, "text/html", 
      "<!DOCTYPE html><html><body><h1>Saved! Rebooting...</h1></body></html>");
    
    delay(2000);
    ESP.restart();
  } else {
    server->send(400, "text/html", 
      "<!DOCTYPE html><html><body><h1>Error: SSID required</h1></body></html>");
  }
}

bool WiFiManager::isConnected() {
  if (apMode) {
    if (server != nullptr) {
      server->handleClient();
    }
    return false;
  }
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::reconnect() {
  if (apMode) return;
  
  if (WiFi.status() != WL_CONNECTED && ssid.length() > 0) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECT_TIMEOUT) {
      delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected!");
    }
  }
}

String WiFiManager::getIP() {
  if (apMode) {
    return WiFi.softAPIP().toString();
  } else {
    return WiFi.localIP().toString();
  }
}

#endif
