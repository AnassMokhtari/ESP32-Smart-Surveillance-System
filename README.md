# ESP32-Smart-Surveillance-System
🔒 A smart IoT home security system using ESP32 &amp; ESP32-CAM | Real-time intrusion detection 📸 | Telegram alerts 🚨 | Keypad &amp; remote deactivation | Open-source

## 📌 Project Overview

This system provides:
- Real-time intrusion detection using ultrasonic sensors
- Automatic photo capture and Telegram alerts
- Remote alarm deactivation via Telegram or physical keypad
- Visual and audible alarms (LCD display, buzzer, LED indicators)

### Key Features
✅ Motion detection with HC-SR04 ultrasonic sensor  
✅ Instant Telegram notifications with intruder photos  
✅ Two-factor deactivation (physical keypad + remote Telegram code)  
✅ Visual feedback via 16×2 LCD display  
✅ Audible alarm with buzzer  

## 🛠 Hardware Components
- **ESP32** (Main controller)
- **ESP32-CAM** (Camera module for photo capture)
- **HC-SR04 Ultrasonic Sensor** (Distance measurement)
- **4×4 Matrix Keypad** (Physical security code input)
- **16×2 LCD with I2C** (System status display)
- **Buzzer** (Audible alarm)
- **LEDs** (Visual indicators)
- **Breadboard & jumper wires**

## 📚 Libraries Used
- [Universal Telegram Bot Library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
- [ArduinoJSON](https://github.com/bblanchon/ArduinoJson)
- [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)
- [Keypad](https://github.com/Chris--A/Keypad)

## 🖥 Software Architecture

### System Components
1. **ESP32-CAM Module** (Camera Node)
   - Handles photo capture
   - Connects to WiFi
   - Sends photos via Telegram bot
   - Runs HTTP server for remote triggering

2. **Main ESP32 Controller** (Alarm Node)
   - Monitors ultrasonic sensor
   - Manages alarm state
   - Handles keypad input
   - Controls LCD display and buzzer
   - Communicates with Telegram bot
  
## 🔌 Wiring Diagram
![Wiring Diagram](docs/Wiring_Diagram.png)
**NOTE** : the arduino is used to upload code to the esp32 cam module using UART protocol since I didnt have the **he ESP32-CAM-MB USB Programmer** . For more detail consult the following video : 
- [Programming of ESP32 Cam Module with Arduino board](https://www.youtube.com/watch?v=k528CUAIgf0&ab_channel=GENIUSTECHNOLOGY)

### 📸 ESP32-CAM Connections
| Component       | ESP32-CAM Pin | Connection Notes              |
|-----------------|---------------|--------------------------------|
| Camera Module   | Internal      | OV2640 built-in               |
| Flash LED       | GPIO 4        | Built-in LED (Active Low)     |
| 5V Power Input  | 5V            | Requires stable 2A power supply|
| Ground          | GND           | Common ground                 |

### 🏠 Main ESP32 Connections
| Component       | ESP32 Pin | Connection Type  | Notes                      |
|----------------|-----------|------------------|----------------------------|
| **Ultrasonic HC-SR04** |          |                  |                            |
| VCC            | 5V        | Power            |                            |
| TRIG           | GPIO 27   | Digital Output   |                            |
| ECHO           | GPIO 36   | Digital Input    |                            |
| GND            | GND       | Ground           |                            |
| **4×4 Keypad** |           |                  |                            |
| ROW1           | GPIO 32   | Digital Output   |                            |
| ROW2           | GPIO 33   | Digital Output   |                            |
| ROW3           | GPIO 25   | Digital Output   |                            |
| ROW4           | GPIO 26   | Digital Output   |                            |
| COL1           | GPIO 4    | Digital Input    | Internal pull-up enabled   |
| COL2           | GPIO 18   | Digital Input    | Internal pull-up enabled   |
| COL3           | GPIO 19   | Digital Input    | Internal pull-up enabled   |
| COL4           | GPIO 2    | Digital Input    | Internal pull-up enabled   |
| **I2C LCD 16×2** |         |                  |                            |
| VCC            | 5V        | Power            |                            |
| GND            | GND       | Ground           |                            |
| SDA            | GPIO 21   | I2C Data         |                            |
| SCL            | GPIO 22   | I2C Clock        |                            |
| **Indicators** |           |                  |                            |
| Buzzer         | GPIO 13   | PWM Output       | Active High               |
| Green LED      | GPIO 14   | Digital Output   | Current limiting resistor |
| Red LED        | GPIO 12   | Digital Output   | Current limiting resistor |
