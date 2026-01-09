# ESP32 Handheld Barcode Scanner

A complete ESP32-based handheld barcode scanner that scans items, prompts for quantity, and sends data to Google Sheets via HTTP POST. Features Wi-Fi configuration, LCD feedback, multiple device support, and automatic retries.

## Features

- **Barcode Scanning**: Serial interface for USB/TTL barcode scanners
- **Quantity Input**: Prompt user for quantity after each scan
- **Google Sheets Integration**: HTTP POST to Google Apps Script Web App
- **Wi-Fi Configuration**: 
  - Access Point (AP) mode for initial setup
  - Station (STA) mode for normal operation
  - Saved credentials in non-volatile storage
- **LCD 20x4 Display**: Real-time feedback for all operations
- **Multi-Device Support**: Unique device ID per ESP32
- **Retry Logic**: Automatic retry on HTTP failures (configurable)
- **Modular Code**: Clean, maintainable header-based architecture

## Hardware Requirements

### Components
- ESP32 development board (ESP32-WROOM-32 or similar)
- LCD 20x4 with I2C interface (PCF8574 backpack)
- USB/TTL barcode scanner (RS232/UART compatible)
- Power supply (USB or battery)

### Wiring

#### LCD 20x4 I2C (default address 0x27)
- SDA → GPIO 21 (ESP32 default SDA)
- SCL → GPIO 22 (ESP32 default SCL)
- VCC → 5V
- GND → GND

#### Barcode Scanner (Serial)
- RX → GPIO 16 (ESP32 TX2)
- TX → GPIO 17 (ESP32 RX2)
- VCC → 5V
- GND → GND

## Software Requirements

### Arduino IDE Libraries
Install these libraries via Arduino Library Manager:

1. **ESP32 Board Support** (v2.0.0 or higher)
   - File > Preferences > Additional Board Manager URLs
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools > Board Manager > Install "esp32"

2. **LiquidCrystal_I2C** (v1.1.2 or higher)
   - By Frank de Brabander

3. **ArduinoJson** (v6.x)
   - By Benoit Blanchon

4. **Preferences** (Built-in with ESP32)

5. **WiFi** (Built-in with ESP32)

6. **HTTPClient** (Built-in with ESP32)

7. **WebServer** (Built-in with ESP32)

## Installation

### 1. Hardware Setup
1. Wire the components as described above
2. Connect ESP32 to your computer via USB

### 2. Arduino IDE Setup
1. Open Arduino IDE
2. Install required libraries (see Software Requirements)
3. Select board: Tools > Board > ESP32 Arduino > ESP32 Dev Module
4. Select port: Tools > Port > (your ESP32 port)

### 3. Upload Code
1. Open `esp32_scanner/esp32_scanner.ino`
2. Review and modify `config.h` if needed (pins, LCD address, etc.)
3. Click Upload

### 4. Google Apps Script Setup
1. Create a new Google Sheet
2. Add two tabs:
   - **Stock**: Headers: `Barcode | Item Name | Quantity | Last Updated`
   - **History**: Headers: `Timestamp | Device ID | Barcode | Quantity | Action | Device Timestamp`
3. Open Tools > Script Editor
4. Paste code from `google_apps_script/Code.gs`
5. Deploy as Web App:
   - Click Deploy > New deployment
   - Select "Web app"
   - Execute as: Your account
   - Who has access: Anyone
6. Copy the deployment URL (you'll need this for ESP32 configuration)

### 5. Wi-Fi Configuration
1. Power on ESP32
2. On first boot, it creates Wi-Fi AP: `ESP32-Scanner-Config`
3. Connect to this network (password: `12345678`)
4. Open browser to `192.168.4.1`
5. Enter:
   - WiFi SSID (your network name)
   - WiFi Password
   - Server URL (Google Apps Script deployment URL)
6. Click "Save and Connect"
7. ESP32 will reboot and connect to your WiFi

## Usage

### Normal Operation
1. **Ready State**: LCD shows "Ready to Scan"
2. **Scan Barcode**: Point scanner at barcode and trigger
3. **LCD Shows Barcode**: Scanned barcode appears on display
4. **Enter Quantity**: Type quantity on Serial Monitor (or via buttons if configured)
5. **Send Data**: ESP32 sends JSON to Google Sheets
6. **Confirmation**: LCD shows "Sent OK!" or "Send Failed!"
7. **Return to Ready**: Device ready for next scan

### Serial Monitor Input
Open Serial Monitor (115200 baud) to:
- See device ID and status messages
- Enter quantity (type number + Enter)
- View HTTP responses
- Debug connection issues

### Example JSON Payload
```json
{
  "deviceId": "ESP32-a1b2c3d4",
  "barcode": "1234567890123",
  "quantity": 5,
  "timestamp": "01:23:45"
}
```

## Configuration

### config.h Options
```cpp
// LCD I2C address (use I2C scanner if different)
#define LCD_I2C_ADDR 0x27

// Scanner serial pins
#define SCANNER_RX_PIN 16
#define SCANNER_TX_PIN 17
#define SCANNER_BAUD 9600

// Wi-Fi AP credentials
#define AP_SSID "you AP name here"
#define AP_PASSWORD "here is your password (optioinal)"

// HTTP retry settings
#define MAX_RETRIES 3
#define RETRY_DELAY 2000  // milliseconds

// Quantity input timeout
#define QUANTITY_TIMEOUT 30000  // milliseconds
```

## Troubleshooting

### LCD Not Working
- Check I2C address with I2C scanner sketch
- Verify wiring (SDA/SCL)
- Adjust contrast potentiometer on I2C backpack

### Barcode Scanner Not Responding
- Check serial wiring (RX/TX)
- Verify baud rate matches scanner (usually 9600)
- Check scanner is in USB/UART mode (not keyboard mode)

### Wi-Fi Connection Failed
- Reset configuration: Re-flash firmware
- Check SSID/password are correct
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

### HTTP POST Failing
- Verify Google Apps Script URL is correct
- Check deployment is set to "Anyone" access
- Enable Serial Monitor to see detailed error messages
- Verify internet connectivity

### Finding LCD I2C Address
Upload and run this sketch to find your LCD address:
```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

void loop() {
  for(byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if(Wire.endTransmission() == 0) {
      Serial.print("Found: 0x");
      Serial.println(i, HEX);
    }
  }
  delay(5000);
}
```

## Multiple Devices

Each ESP32 gets a unique device ID based on its MAC address. You can deploy multiple scanners:

1. Flash the same code to multiple ESP32s
2. Each will have a unique device ID (e.g., `ESP32-a1b2c3d4`)
3. Configure each with the same Google Apps Script URL
4. Google Sheet History tab will show which device made each entry

## Code Structure

```
esp32_scanner/
├── esp32_scanner.ino          # Main Arduino sketch
├── config.h                   # Configuration settings
├── wifi_manager.h             # Wi-Fi AP/STA management
├── scanner_manager.h          # Barcode scanner interface
├── display_manager.h          # LCD display functions
└── http_client_manager.h      # HTTP POST handling

google_apps_script/
└── Code.gs                    # Google Apps Script backend
```

## API Reference

### WiFiManager
- `begin()` - Initialize Wi-Fi, load credentials, connect or start AP
- `isConnected()` - Check if connected to Wi-Fi
- `reconnect()` - Attempt to reconnect to saved network

### ScannerManager
- `begin()` - Initialize serial connection to scanner
- `available()` - Check if barcode data is available
- `read()` - Read scanned barcode string

### DisplayManager
- `showStartup()` - Show startup screen with device ID
- `showReady()` - Show ready to scan message
- `showBarcode()` - Display scanned barcode
- `showQuantityPrompt()` - Prompt for quantity input
- `showStatus()` - Show status message
- `showError()` - Show error message
- `showSuccess()` - Show success message

### HTTPClientManager
- `begin()` - Load server URL from preferences
- `sendData()` - POST JSON payload to server

## Future Enhancements

- [ ] NTP time synchronization for accurate timestamps
- [ ] Button interface for quantity input (no serial needed)
- [ ] OLED display support
- [ ] Battery monitoring
- [ ] Deep sleep mode for battery saving
- [ ] Local data buffering when offline
- [ ] OTA (Over-The-Air) firmware updates
- [ ] BLE configuration alternative

## License

MIT License - feel free to use and modify for your projects.

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Support

For issues, questions, or suggestions, please open an issue on GitHub.

## Author

SA1933

## Version History

- **v1.0** (2024) - Initial release
  - Basic scanning, quantity input, HTTP POST
  - Wi-Fi AP/STA configuration
  - LCD 20x4 support
  - Modular architecture
