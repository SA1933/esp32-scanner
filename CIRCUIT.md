# ESP32 Barcode Scanner - Circuit Diagram

## Wiring Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                         ESP32 DevKit                            │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                                                          │  │
│  │  3V3 ────────────────────────────────────────────────    │  │
│  │                                                          │  │
│  │  GND ────────┬──────────┬──────────────────────────      │  │
│  │              │          │                               │  │
│  │  GPIO21(SDA)─┼──────────┼──────────────────────────     │  │
│  │              │          │                               │  │
│  │  GPIO22(SCL)─┼──────────┼──────────────────────────     │  │
│  │              │          │                               │  │
│  │  GPIO16(TX2)─┼──────────┼──────────────────────────     │  │
│  │              │          │                               │  │
│  │  GPIO17(RX2)─┼──────────┼──────────────────────────     │  │
│  │              │          │                               │  │
│  │  5V ─────────┼──────────┼──────────────────────────     │  │
│  │              │          │                               │  │
│  └──────────────┼──────────┼──────────────────────────────┘  │
│                 │          │                                  │
└─────────────────┼──────────┼──────────────────────────────────┘
                  │          │
                  │          │
    ┌─────────────┴───┐   ┌──┴────────────────┐
    │                 │   │                   │
    │  LCD 20x4 I2C   │   │  Barcode Scanner  │
    │  (PCF8574)      │   │  (UART/TTL)       │
    │                 │   │                   │
    │  VCC ← 5V       │   │  VCC ← 5V         │
    │  GND ← GND      │   │  GND ← GND        │
    │  SDA ← GPIO21   │   │  TX  ← GPIO17     │
    │  SCL ← GPIO22   │   │  RX  ← GPIO16     │
    │                 │   │                   │
    └─────────────────┘   └───────────────────┘
```

## Pin Assignments

### ESP32 to LCD 20x4 I2C
| ESP32 Pin | LCD Pin | Function      |
|-----------|---------|---------------|
| 5V        | VCC     | Power         |
| GND       | GND     | Ground        |
| GPIO21    | SDA     | I2C Data      |
| GPIO22    | SCL     | I2C Clock     |

### ESP32 to Barcode Scanner (UART)
| ESP32 Pin | Scanner Pin | Function      |
|-----------|-------------|---------------|
| 5V        | VCC         | Power         |
| GND       | GND         | Ground        |
| GPIO16    | RX          | Serial TX     |
| GPIO17    | TX          | Serial RX     |

## Component Details

### LCD 20x4 with I2C
- Module: 2004 LCD with PCF8574 I2C backpack
- I2C Address: 0x27 (or 0x3F, check with I2C scanner)
- Voltage: 5V
- Backlight: Built-in with brightness control

### Barcode Scanner
- Type: USB/TTL barcode scanner (1D/2D compatible)
- Interface: UART/Serial (not USB keyboard mode)
- Baud Rate: 9600 (configurable)
- Output: ASCII characters with CR/LF terminator
- Examples: GM65, GM66, LS1203, IT3100

### ESP32 Development Board
- Board: ESP32-WROOM-32 or ESP32-DevKitC
- Flash: 4MB minimum
- WiFi: 2.4GHz 802.11 b/g/n
- Power: 5V via USB or VIN pin

## Power Considerations

### Power Supply Options
1. **USB Power**: 5V from USB port (500mA minimum)
2. **Battery**: 5V power bank or 3.7V LiPo with boost converter
3. **Wall Adapter**: 5V 1A DC adapter

### Current Consumption (Typical)
- ESP32: 80-160mA (WiFi active)
- LCD 20x4: 100-150mA (backlight on)
- Barcode Scanner: 100-200mA (scanning)
- **Total**: ~400mA maximum

### Battery Life Estimation
- 2000mAh power bank: ~4-5 hours continuous use
- 5000mAh power bank: ~10-12 hours continuous use
- 10000mAh power bank: ~20-24 hours continuous use

## I2C Address Detection

If your LCD doesn't work with default address 0x27, run this I2C scanner:

```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int devices = 0;
  
  Serial.println("Scanning...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }
  
  if (devices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Done\n");
  
  delay(5000);
}
```

## Notes

- All ground connections must be common
- Use proper 5V power supply for stable operation
- Keep I2C wires short (<30cm) to avoid noise
- Use shielded cable for barcode scanner if possible
- Add 0.1uF capacitor near ESP32 VCC/GND for stability
- LCD contrast: Adjust potentiometer on I2C backpack if needed
