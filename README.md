# IoT Anti-Theft Detection System — ESP32-CAM

An embedded IoT security system that detects motion via a PIR sensor,
captures a JPEG image using the ESP32-CAM, and instantly sends a
photo alert to your phone via **Telegram Bot API** over Wi-Fi.

---

## Features

- PIR motion detection with configurable cooldown
- Real-time JPEG image capture on trigger
- Telegram photo alert with location caption
- Text-only fallback if camera capture fails
- LED status indicators for connection and trigger state
- Auto Wi-Fi reconnect handling

---

## Hardware

| Component        | Details                        |
|------------------|--------------------------------|
| ESP32-CAM        | AI-Thinker (OV2640 camera)     |
| PIR Sensor       | HC-SR501                       |
| FTDI Programmer  | CP2102 / CH340 for flashing    |
| Power Supply     | 5V / 2A                        |
| Status LED       | 3.3V LED on GPIO33 (optional)  |

---

## Circuit
```
ESP32-CAM    HC-SR501
─────────    ────────
GPIO13   ──  OUT
GND      ──  GND
5V       ──  VCC

ESP32-CAM    Status LED
─────────    ──────────
GPIO33   ──[330Ω]── Anode
GND      ────────── Cathode
```

---

## Setup

### 1. Arduino IDE

- Add board URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Install ESP32 board package
- Select Board: `AI Thinker ESP32-CAM`
- Partition Scheme: `Huge APP (3MB No OTA)`

### 2. Clone
```bash
git clone https://github.com/dkdarshan95/esp32-antitheft-detection.git
cd esp32-antitheft-detection
```

### 3. Configure
```bash
cp config_example.h config.h
```

Edit `config.h` with your Wi-Fi and Telegram credentials.

### 4. Get Telegram Credentials

1. Message **@BotFather** → `/newbot` → copy token
2. Message **@userinfobot** → `/start` → copy chat ID

### 5. Flash

1. Connect FTDI: TX→UOR, RX→UOT, GND→GND, VCC→5V
2. Tie IO0 → GND (flash mode)
3. Press Reset → Upload in Arduino IDE
4. Disconnect IO0 → GND → Press Reset

---

## Project Structure
```
esp32-antitheft-detection/
├── esp32_antitheft.ino    Main sketch
├── camera_config.h        Camera pin map & init
├── telegram_notify.h      Telegram alert functions
├── config.h               Your credentials (git-ignored)
├── config_example.h       Safe template to commit
├── .gitignore
└── README.md
```

---

## How It Works
```
PIR detects motion
      ↓
ESP32-CAM wakes & checks cooldown
      ↓
Captures JPEG with flash
      ↓
HTTPS POST → api.telegram.org/sendPhoto
      ↓
Photo + alert caption → Your phone
```

---

## Troubleshooting

| Symptom | Fix |
|---|---|
| Camera init failed | Use 5V/2A supply; press Reset |
| Wi-Fi not connecting | Check SSID/password; use 2.4GHz |
| No Telegram message | Verify bot token and chat ID |
| Brownout reset loop | Better power supply needed |
| Blurry image | Clean lens; increase CAPTURE_DELAY_MS |

---

## Future Improvements

- [ ] SD card local image storage
- [ ] Web dashboard via HTTP server
- [ ] Deep sleep for battery operation
- [ ] MQTT / email notifications
- [ ] Face detection with ESP-WHO

---

## Author

**DK Darshan ** — [github.com/dkdarshan95](https://github.com/dkdarshan95)

## License

MIT License
