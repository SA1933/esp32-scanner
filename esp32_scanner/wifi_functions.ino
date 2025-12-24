void connectWiFi(String ssid, String pass) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid.substring(0, 20));
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  unsigned long start = millis();
  int dots = 0;
  
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
    delay(500);
    lcd.setCursor(dots, 2);
    lcd.print(".");
    dots = (dots + 1) % LCD_COLS;
   
    char key = readKeypad();
    if (key == 'C') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connection cancelled");
      delay(2000);
      displayReset();
      return;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.SSID());
    lcd.setCursor(0, 2);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP().toString());
    delay(2000);

    server.on("/", HTTP_GET, []() {
      String html = "<!DOCTYPE html><html><head><title>Scanner Control</title>";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>body{font-family:Arial,sans-serif;margin:20px;}</style>";
      html += "</head><body>";
      html += "<h1>ESP32 Scanner</h1>";
      html += "<p>Connected to: " + WiFi.SSID() + "</p>";
      html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
      html += "<p><a href='/reset' style='color:red;'>Reset WiFi Settings</a></p>";
      html += "</body></html>";
      server.send(200, "text/html", html);
    });
    
    server.on("/reset", HTTP_GET, []() {
      String html = "<!DOCTYPE html><html><body>";
      html += "<h1>Resetting WiFi...</h1>";
      html += "<p>Clearing credentials and restarting.</p>";
      html += "</body></html>";
      server.send(200, "text/html", html);
      
      prefs.begin("wifi", false);
      prefs.clear();
      prefs.end();
      delay(1000);
      ESP.restart();
    });
    
    server.begin();
    Serial.println("HTTP server started");
    Serial.println("IP address: " + WiFi.localIP().toString());

    displayReset();
    
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Starting AP Mode");
    delay(2000);
    startAP();
  }
}

void resetWiFiSettings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset WiFi...");
  lcd.setCursor(0, 1);
  lcd.print("Clearing creds");

  prefs.begin("wifi", false);
  prefs.clear();
  prefs.end();

  delay(1000);
  lcd.setCursor(0, 2);
  lcd.print("Rebooting...");
  delay(500);
  ESP.restart();
}

void startAP() {
  apMode = true;
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AP Mode");
  lcd.setCursor(0, 1);
  lcd.print("SSID: ");
  lcd.print(AP_SSID);
  lcd.setCursor(0, 2);
  lcd.print("IP: ");
  lcd.print(IP.toString());
  lcd.setCursor(0, 3);
  lcd.print("Open ");
  if (String(AP_PASSWORD).length() == 0) lcd.print("(no pass)");

  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>WiFi Setup</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:20px;}";
    html += ".network{cursor:pointer;padding:10px;border:1px solid #ddd;margin:5px 0;}";
    html += ".network:hover{background:#f0f0f0;}";
    html += "input{padding:10px;margin:5px 0;width:100%;box-sizing:border-box;}";
    html += "button{padding:10px 20px;background:#4CAF50;color:white;border:none;cursor:pointer;}";
    html += "</style>";
    html += "<script>";
    html += "function selectNetwork(ssid){";
    html += "  document.getElementById('ssid').value = ssid;";
    html += "  document.getElementById('ssid').focus();";
    html += "}";
    html += "</script>";
    html += "</head><body>";
    html += "<h1>WiFi Configuration</h1>";
    html += "<p>Available Networks:</p>";

    int n = WiFi.scanNetworks();
    if (n == 0) {
      html += "<p>No networks found</p>";
    } else {
      for (int i = 0; i < n; i++) {
        html += "<div class='network' onclick=\"selectNetwork('" + WiFi.SSID(i) + "')\">";
        html += WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)";
        html += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " Open" : " Secured";
        html += "</div>";
      }
    }
    
    html += "<form action='/save' method='POST'>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='SSID' required><br>";
    html += "<input type='password' name='pass' placeholder='Password'><br>";
    html += "<button type='submit'>Save & Connect</button>";
    html += "</form>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
  });
  
  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    
    if (ssid.length() > 0) {
      prefs.begin("wifi", false);
      prefs.putString("ssid", ssid);
      prefs.putString("pass", pass);
      prefs.end();
      
      String html = "<!DOCTYPE html><html><body>";
      html += "<h1>Credentials Saved!</h1>";
      html += "<p>SSID: " + ssid + "</p>";
      html += "<p>Restarting to connect to WiFi...</p>";
      html += "</body></html>";
      
      server.send(200, "text/html", html);
      
      Serial.println("WiFi credentials saved:");
      Serial.println("SSID: " + ssid);
      Serial.println("Password: " + pass);
      
      delay(3000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "SSID cannot be empty");
    }
  });
  
  server.begin();
  Serial.println("AP mode web server started");
}
