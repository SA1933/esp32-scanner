# Quick Start Guide - ESP32 Barcode Scanner

## Step-by-Step Setup (15 Minutes)

### Part 1: Google Sheets Setup (5 min)

1. **Create Google Sheet**
   - Go to [Google Sheets](https://sheets.google.com)
   - Create new spreadsheet named "Barcode Inventory"

2. **Create Stock Tab**
   - Rename "Sheet1" to "Stock"
   - Add headers in row 1:
     - A1: `Barcode`
     - B1: `Item Name`
     - C1: `Quantity`
     - D1: `Last Updated`

3. **Create History Tab**
   - Click "+" to add new sheet
   - Name it "History"
   - Add headers in row 1:
     - A1: `Timestamp`
     - B1: `Device ID`
     - C1: `Barcode`
     - D1: `Quantity`
     - E1: `Action`
     - F1: `Device Timestamp`

4. **Deploy Script**
   - Click Extensions > Apps Script
   - Delete default code
   - Copy paste from `google_apps_script/Code.gs`
   - Click Deploy > New deployment
   - Click gear icon next to "Select type"
   - Choose "Web app"
   - Settings:
     - Description: "ESP32 Scanner API"
     - Execute as: "Me"
     - Who has access: "Anyone"
   - Click Deploy
   - Authorize the script (click "Authorize access")
   - Copy the Web app URL (save for later)

### Part 2: Hardware Assembly (5 min)

1. **Connect LCD 20x4**
   ```
   LCD I2C Module → ESP32
   VCC → 5V
   GND → GND
   SDA → GPIO21
   SCL → GPIO22
   ```

2. **Connect Barcode Scanner**
   ```
   Scanner → ESP32
   VCC → 5V
   GND → GND
   TX  → GPIO17 (RX2)
   RX  → GPIO16 (TX2)
   ```

3. **Connect Power**
   - Plug ESP32 into computer via USB

### Part 3: Software Setup (5 min)

1. **Install Arduino IDE**
   - Download from [arduino.cc](https://www.arduino.cc/en/software)
   - Install and open

2. **Add ESP32 Board**
   - File > Preferences
   - Additional Board Manager URLs:
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Tools > Board > Boards Manager
   - Search "esp32"
   - Install "esp32 by Espressif Systems"

3. **Install Libraries**
   - Tools > Manage Libraries
   - Install these libraries:
     - `LiquidCrystal_I2C` by Frank de Brabander
     - `ArduinoJson` by Benoit Blanchon (v6.x)

4. **Upload Code**
   - Open `esp32_scanner/esp32_scanner.ino`
   - Tools > Board > ESP32 Arduino > ESP32 Dev Module
   - Tools > Port > (select your ESP32 port)
   - Click Upload button
   - Wait for "Done uploading"

### Part 4: Wi-Fi Configuration (2 min)

1. **Connect to ESP32 AP**
   - Look at LCD - should show "Config Mode"
   - On phone/computer WiFi:
     - Network: `ESP32-Scanner-Config`
     - Password: `12345678`

2. **Configure Settings**
   - Open browser to `192.168.4.1`
   - Enter:
     - WiFi SSID: Your home/office WiFi name
     - Password: Your WiFi password
     - Server URL: Google Apps Script URL from Part 1
   - Click "Save and Connect"

3. **Verify Connection**
   - ESP32 will reboot
   - LCD should show "WiFi Connected!" then "Ready to Scan"
   - If failed, repeat from step 1

### Part 5: First Scan

1. **Test Scanner**
   - Point barcode scanner at any barcode
   - Press scanner trigger
   - LCD should show the barcode number

2. **Enter Quantity**
   - Open Arduino IDE Serial Monitor (Tools > Serial Monitor)
   - Set baud rate to 115200
   - Type a number (e.g., `5`) and press Enter
   - LCD should show "Sending..." then "Sent OK!"

3. **Check Google Sheet**
   - Refresh your Google Sheet
   - Stock tab: Should show new row with barcode and quantity
   - History tab: Should show transaction log

## Troubleshooting Quick Fixes

### LCD shows nothing
- Check I2C address in `config.h` (try 0x27 or 0x3F)
- Adjust contrast knob on I2C backpack
- Verify wiring

### Scanner not working
- Check scanner is in UART mode (not keyboard mode)
- Verify baud rate (usually 9600)
- Swap TX/RX wires if needed

### Can't connect to WiFi
- Make sure WiFi is 2.4GHz (not 5GHz)
- Check SSID and password
- Move ESP32 closer to router

### "Send Failed" error
- Verify Google Apps Script URL is correct
- Check deployment is set to "Anyone" access
- Make sure ESP32 has internet access

## Next Steps

- Add more items to scan
- Customize item names in Google Sheet
- Add multiple ESP32 devices
- Modify code for your needs

## Support

See main README.md for detailed documentation.
Issues? Create an issue on GitHub.

---
Ready to scan! 🎉
