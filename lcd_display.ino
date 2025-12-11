void displaySKULocked() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SKU SCANNED:");
  lcd.setCursor(0, 1);
  lcd.print(sku.substring(0, 20));
  lcd.setCursor(0, 2);
  lcd.print("Qty: ");
  lcd.setCursor(0, 3);
  lcd.print("B:(-) A:<- D:Enter");
  
  Serial.print("SKU: ");
  Serial.print(sku);
  Serial.print(" | Qty: ");
}

void updateQtyDisplay() {
  lcd.setCursor(5, 2);
  lcd.print("                ");
  lcd.setCursor(5, 2);
  if (negative) lcd.print("-");
  lcd.print(input);
  
  Serial.print("\rSKU: ");
  Serial.print(sku);
  Serial.print(" | Qty: ");
  if (negative) Serial.print("-");
  Serial.print(input);
  for (int i = input.length(); i < 6; i++) Serial.print(" ");
}

void displayReset() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready to Scan");
  lcd.setCursor(0, 1);
  lcd.print("Scan SKU Barcode");
  lcd.setCursor(0, 2);
  lcd.print("WiFi: ");
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("Connected");
    lcd.setCursor(0, 3);
    lcd.print("IP: ");
    String ip = WiFi.localIP().toString();
    lcd.print(ip.substring(0, 16));
  } else {
    lcd.print("Disconnected");
  }
}

void displaySending() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending Data...");
  lcd.setCursor(0, 1);
  lcd.print("SKU: " + sku.substring(0, 15));
  lcd.setCursor(0, 2);
  lcd.print("Qty: ");
  if (negative) lcd.print("-");
  lcd.print(input);
}

void displaySent() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Data Sent!");
  lcd.setCursor(0, 1);
  lcd.print("SKU: " + sku.substring(0, 15));
  lcd.setCursor(0, 2);
  lcd.print("Qty: ");
  if (negative) lcd.print("-");
  lcd.print(input);
  delay(1000);
  displayReset();
}
