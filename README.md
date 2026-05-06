# Room Comfort Monitor with Auto Fan Control

An Arduino-based embedded system that monitors room temperature and humidity and automatically adjusts fan speed to keep elderly residents safe from heat stress. A servo motor simulates the fan, an OLED screen shows live readings, and an RGB LED gives an instant green/yellow/red comfort status.

# SIT210 Embedded Systems Development



# Simulation (Proof of Concept)

The Wokwi simulation was built as a **proof of concept and schematic guide** before physical assembly. It confirmed all pin assignments, I2C wiring, comfort zone logic, and servo behaviour were correct before any hardware was connected.

**To run the simulation** Go to [wokwi.com](https://wokwi.com), create a new Arduino Uno project, paste in `diagram.json` and `sketch.ino`, then click **Start Simulation**. Click the DHT22 component and drag its temperature/humidity sliders to test the system's response.



# Physical Prototype

# Components

| Component | Quantity |
|---|---|
| Arduino Uno R3 | 1 |
| DHT22 Temperature & Humidity Sensor | 1 |
| SSD1306 OLED Display (128×64, I2C) | 1 |
| Servo Motor | 1 |
| RGB LED (common cathode) | 1 |
| Resistor 10kΩ | 1 |
| Resistor 220Ω | 2 |
| Breadboard | 1 |
| Jumper wires | — |


# Wiring
The wiring is done using the same configuration as the schematic design (wokwi simulation) shown below

![image alt]()

| Component | Pin | Arduino Pin |
|---|---|---|
| DHT22 | VCC | 5V |
| DHT22 | DATA | D2 |
| DHT22 | GND | GND |
| 10kΩ resistor | Between DATA and 5V | (pull-up) |
| SSD1306 OLED | VCC | 3.3V |
| SSD1306 OLED | GND | GND |
| SSD1306 OLED | SDA | A4 |
| SSD1306 OLED | SCL | A5 |
| Servo | Signal (orange) | D9 |
| Servo | VCC (red) | 5V |
| Servo | GND (brown) | GND |
| RGB LED | R anode | D10 (via 220Ω) |
| RGB LED | G anode | D11 (via 220Ω) |
| RGB LED | Common cathode | GND |

> **RGB LED note:** Set `common` to `cathode` in the RGB LED component. HIGH = on, LOW = off.



# Comfort Zones

| Zone | Temperature | Fan (Servo) | LED |
|---|---|---|---|
| COOL | < 24°C | 0° — Off | 🟢 Green |
| MED-LOW | 24–26°C | 45° — Low | 🟢 Green |
| MED | 26–28°C | 90° — Medium | 🟡 Yellow |
| MED-HIGH | 28–30°C | 135° — High | 🟡 Yellow |
| HOT | > 30°C | 180° — Full | 🔴 Red |



# Libraries Required

Install these via **Arduino IDE → Tools → Manage Libraries**:

- `DHT sensor library` — Adafruit
- `Adafruit Unified Sensor`
- `Adafruit SSD1306`
- `Adafruit GFX Library`
- `Servo` — built into Arduino IDE



# How to Run on the Physical Prototype

1. Wire up all components as shown in the table above.
2. Open `prototype/RoomComfortMonitor.ino` in the Arduino IDE.
3. Install the required libraries listed above.
4. Select **Tools → Board → Arduino Uno** and the correct COM port.
5. Click **Upload**.
6. Once uploaded, the OLED will show a startup screen, then begin displaying live temperature and humidity readings.
7. The servo will move and the LED will change colour automatically as the room temperature changes.
8. To test without changing the actual room temperature, briefly hold a warm object close to the DHT22 sensor.



# How It Works

The Arduino polls the DHT22 every 2 seconds using `millis()`-based non-blocking scheduling — no `delay()` calls are used anywhere in the prototype code. The reading is passed to `determineZone()`, which returns one of five comfort levels. That single result drives all three outputs: the servo angle, the LED colour, and the OLED status line — all updated in the same loop cycle.

If the DHT22 returns an invalid reading, the OLED displays a sensor error message and the LED blinks red until the next successful read.



