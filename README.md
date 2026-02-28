# ATHERMO Custom Component for ESPHome

**Author:** Mirosław Kardaś (ATNEL)  
**Purpose:** Power management and PIR control for ATB-THERMO weather station

## Features

✅ **PIR_DIS control** - Disconnects PIR from ESP RST pin during boot (prevents spurious resets)  
✅ **PERIPH_VCC control** - P-MOSFET power management for OLED and sensors  
✅ **Power cycling** - Hard reset for frozen peripherals  
✅ **Hardware priority** - Runs very early in boot sequence (setup_priority::HARDWARE = 600)  
✅ **Simple configuration** - Always boots with peripherals powered ON

## Hardware Details

- **PIR_DIS (GPIO15):** Controls 74LVC1G3157DW analog switch
  - **Always HIGH on boot** - PIR disconnected from ESP RST
  - Prevents reset loops from PIR during operation
  
- **PERIPH_VCC (GPIO0):** Controls P-MOSFET for peripheral power
  - **Always LOW on boot** - Power ON for peripherals (OLED, BME280, BH1750, DS18B20)
  - LOW = P-MOSFET conducts = Power ON
  - HIGH = P-MOSFET off = Power OFF (for deep sleep power saving)

- **PIR_CHECK (GPIO14):** Direct PIR signal monitoring (not managed by this component)
  - Use standard ESPHome `binary_sensor` platform for this pin

## Installation

Copy the `athermo` folder to your project's `components` directory:

```
your-project/
├── your-project.yaml
└── components/
    └── athermo/           ← place component here
        ├── __init__.py
        ├── athermo.h
        └── athermo.cpp
```

## Configuration

```yaml
external_components:
  - source:
      type: local
      path: components

athermo:
  pir_dis_pin: GPIO15      # PIR disconnect control
  periph_vcc_pin: GPIO0    # P-MOSFET power control
```

**That's it!** No additional parameters needed. Component always boots with:
- PIR_DIS = HIGH (PIR disconnected)
- PERIPH_VCC = LOW (peripherals powered ON)

## Available Actions

### `athermo.periph_vcc_on`
Turn peripheral power ON (OLED, sensors).

```yaml
button:
  - platform: template
    name: "Power On Peripherals"
    on_press:
      - athermo.periph_vcc_on
```

### `athermo.periph_vcc_off`
Turn peripheral power OFF (power saving mode).

```yaml
deep_sleep:
  on_shutdown:
    - athermo.periph_vcc_off  # Save power during sleep
```

### `athermo.power_cycle`
Power cycle peripherals (OFF → delay → ON). Useful for resetting frozen OLED or sensors.

```yaml
button:
  - platform: template
    name: "Reset OLED"
    on_press:
      - athermo.power_cycle:
          delay_ms: 500  # Optional, default is 500ms
```

## Complete Example - ATB-THERMO Weather Station

```yaml
esphome:
  name: atb-thermo
  platform: ESP8266
  board: esp_wroom_02

external_components:
  - source:
      type: local
      path: components

# ATHERMO component - boots with peripherals powered ON
athermo:
  pir_dis_pin: GPIO15
  periph_vcc_pin: GPIO0

# I2C bus for sensors and OLED
i2c:
  sda: GPIO4
  scl: GPIO5
  frequency: 400kHz

# OLED display
display:
  - platform: ssd1306_i2c
    model: "SSD1306 128x64"
    address: 0x3C
    id: oled_display
    # Display will work immediately on boot since peripherals are powered

# Sensors
sensor:
  - platform: bme280
    address: 0x76
    temperature:
      name: "Temperature"
    humidity:
      name: "Humidity"
    pressure:
      name: "Pressure"

# PIR motion detection
binary_sensor:
  - platform: gpio
    pin: GPIO14
    name: "PIR Motion"
    id: pir_check
    device_class: motion
    on_press:
      - logger.log: "Motion detected!"
      - script.execute: display_mode

# Deep sleep with power management
deep_sleep:
  id: deep_sleep_control
  run_duration: 5s
  sleep_duration: 10min
  on_shutdown:
    # Power off peripherals before sleep to save energy
    - athermo.periph_vcc_off
    - logger.log: "Powering off peripherals, entering deep sleep..."

# Display mode script
script:
  - id: display_mode
    mode: restart
    then:
      # Peripherals are already ON, just show data
      - display.page.show: weather_page
      - delay: 30s
      - deep_sleep.enter: deep_sleep_control

# Maintenance buttons
button:
  - platform: template
    name: "Reset OLED"
    on_press:
      - athermo.power_cycle:
          delay_ms: 500

  - platform: template
    name: "Power Off Peripherals"
    on_press:
      - athermo.periph_vcc_off

  - platform: template
    name: "Power On Peripherals"
    on_press:
      - athermo.periph_vcc_on
```

## Typical Use Cases

### 1. Power Saving During Deep Sleep
```yaml
deep_sleep:
  on_shutdown:
    - athermo.periph_vcc_off  # Turn off OLED and sensors
```

### 2. OLED Frozen - Hard Reset
```yaml
button:
  - platform: template
    name: "Fix Frozen OLED"
    on_press:
      - athermo.power_cycle  # OFF → 500ms → ON
```

### 3. Motion-Activated Display
```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO14
    on_press:
      - display.page.show: main_page
      - delay: 30s
      - athermo.periph_vcc_off  # Save power after timeout
```

### 4. Sensor Measurement Only When Needed
```yaml
interval:
  - interval: 10min
    then:
      - athermo.periph_vcc_on
      - delay: 100ms  # Wait for sensors to stabilize
      - component.update: bme280_sensor
      - delay: 1s
      - athermo.periph_vcc_off  # Power off to save energy
```

## Boot Sequence

1. **ESP starts** (from deep sleep or reset)
2. **ATHERMO setup() runs early** (HARDWARE priority = 600)
   - GPIO15 → HIGH (PIR disconnected)
   - GPIO0 → LOW (peripherals powered ON)
3. **Other components initialize** (display, sensors, etc.)
   - All peripherals have power, no timeouts
4. **Normal operation** begins

## Technical Notes

- **Setup Priority:** `setup_priority::HARDWARE (600)` - runs very early
- **PIR_DIS:** Always set to HIGH on boot to prevent reset loops from PIR
- **PERIPH_VCC:** Always set to LOW on boot (power ON) for smooth initialization
- **P-MOSFET Logic:** Inverted (LOW = ON, HIGH = OFF)
- **Power Cycle Default:** 500ms delay (configurable)

## Troubleshooting

**Q: ESP keeps resetting in a loop**  
A: Check that `pir_dis_pin` is correctly set to GPIO15 and component is loaded

**Q: OLED doesn't initialize**  
A: Component should set PERIPH_VCC LOW automatically. Check wiring and P-MOSFET

**Q: Display timeout on boot (15+ seconds)**  
A: This was fixed - component now always boots with peripherals ON

**Q: Sensors frozen after deep sleep**  
A: Add `athermo.power_cycle` after wake-up to hard reset peripherals

**Q: High power consumption in deep sleep**  
A: Make sure `periph_vcc_off` is called in `on_shutdown` before sleep

## Power Consumption Reference

| Mode | PERIPH_VCC | Consumption |
|------|-----------|-------------|
| Boot / Normal | ON (LOW) | ~23-68mA depending on WiFi |
| Deep Sleep | OFF (HIGH) | ~120µA (with PIR) |

With proper power management (OFF during sleep), a 3000mAh battery can last weeks or months depending on measurement frequency.

## Component Files Structure

```
components/
└── athermo/
    ├── __init__.py      # Python configuration and code generation
    ├── athermo.h        # C++ header with class definitions
    └── athermo.cpp      # C++ implementation (boot: GPIO15=HIGH, GPIO0=LOW)
```

## License

Created for ATB-THERMO project by Mirosław Kardaś (ATNEL)  
Website: https://ehs.atnel.pl  
Academy: https://akademia.atnel.pl

---

**For support and updates, visit:** https://ehs.atnel.pl
