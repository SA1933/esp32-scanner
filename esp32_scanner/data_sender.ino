#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

extern LiquidCrystal_I2C lcd;
extern bool apMode;
extern String sku;
extern String input;
extern bool waitingQty;
extern bool negative;
extern String itemNameShort;
extern String itemNameShort2;
extern String itemNameShort3;
extern long currentStock;
void displayReset();
void displaySent();

static String urlEncode(const String &s) {
  String out = "";
  const char *hex = "0123456789ABCDEF";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      out += c;
    } else {
      out += '%';
      out += hex[(c >> 4) & 0xF];
      out += hex[c & 0xF];
    }
  }
  return out;
}

void fetchItemInfo(const String &skuCode) {
  itemNameShort = "";
  itemNameShort2 = "";
  itemNameShort3 = "";
  currentStock = 0;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("fetchItemInfo: WiFi not connected");
    return;
  }

  HTTPClient http;
  String url = String(GAS_URL) + String("?sku=") + urlEncode(skuCode) + String("&info=1");
  http.setConnectTimeout(8000);

  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.begin(url);
  int code = http.GET();
  Serial.print("fetchItemInfo GET "); Serial.print(url); Serial.print(" -> code "); Serial.println(code);
  if (code == 200) {
    String body = http.getString();
    size_t previewLen = body.length();
    if (previewLen > 200) previewLen = 200;
    Serial.print("body: "); Serial.println(body.substring(0, (int)previewLen));
    // Parse fields
    int is = body.indexOf("\"short\":");
    if (is >= 0) {
      int q1 = body.indexOf('"', is + 8);
      int q2 = (q1 >= 0) ? body.indexOf('"', q1 + 1) : -1;
      if (q1 >= 0 && q2 > q1) itemNameShort = body.substring(q1 + 1, q2);
    }
    int i2 = body.indexOf("\"short2\":");
    if (i2 >= 0) {
      int q1 = body.indexOf('"', i2 + 9);
      int q2 = (q1 >= 0) ? body.indexOf('"', q1 + 1) : -1;
      if (q1 >= 0 && q2 > q1) itemNameShort2 = body.substring(q1 + 1, q2);
    }
    int i3 = body.indexOf("\"short3\":");
    if (i3 >= 0) {
      int q1 = body.indexOf('"', i3 + 9);
      int q2 = (q1 >= 0) ? body.indexOf('"', q1 + 1) : -1;
      if (q1 >= 0 && q2 > q1) itemNameShort3 = body.substring(q1 + 1, q2);
    }
    int it = body.indexOf("\"stock\":");
    if (it >= 0) {
      int c1 = body.indexOf(':', it);
      int c2 = (c1 >= 0) ? body.indexOf(',', c1) : -1;
      String num = (c1 >= 0) ? body.substring(c1 + 1, c2 > 0 ? c2 : body.length()) : "0";
      num.trim();
      currentStock = num.toInt();
    }
    if (itemNameShort2.length() == 0 && itemNameShort.length() > 0) {
      itemNameShort2 = itemNameShort;
    }
    Serial.print("parsed short2='"); Serial.print(itemNameShort2);
    Serial.print("' short3='"); Serial.print(itemNameShort3);
    Serial.print("' stock="); Serial.println(currentStock);
  } else {
    Serial.println("fetchItemInfo: HTTP error");
  }
  http.end();
}

void sendData(String skuParam, String qty) {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Disconnected!");
    lcd.setCursor(0, 1);
    lcd.print("Cannot send data");
    delay(2000);
    displayReset();
    return;
  }

  String ipStr = String("192.168.4.1");
  String jenisIp = apMode ? "AP" : "STA";

  int qtyVal = qty.toInt();
  String ops = (qtyVal >= 0) ? "in" : "out";
  
  HTTPClient http;
  http.begin(GAS_URL);
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"sku\":\"" + skuParam + "\",\"jumlah\":" + qty +
                   ",\"ip\":\"" + ipStr + "\",\"jenis_ip\":\"" + jenisIp +
                   "\",\"ops\":\"" + ops + "\"}";
  int code = http.POST(payload);
  
  if (code == 200) {
    displaySent();
    Serial.println("Success");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Send Error: ");
    lcd.print(code);
    delay(2000);
    displayReset();
  }
  
  http.end();
  
  sku = "";
  waitingQty = false;
  input = "";
  negative = false;
}
