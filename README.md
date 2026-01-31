# ESP8266-Telegram-Sensor-Node
This project turns an ESP-12F (ESP8266) into a WiFi-connected environmental monitoring node that reports sensor data via Telegram.


📦 Features
📡 WiFi connectivity
💬 Telegram bot control
🌡 Temperature (AHT20)
💧 Humidity (AHT20)
🌬 Barometric Pressure (BMP280)
🔥 Gas detection (MQ-2)

LPG
Carbon Monoxide (CO)
Smoke

📊 Daily automatic report (12:00 PM)
🧾 On-demand sensor queries via Telegram
🧠 Supported Telegram Commands

Send these messages to your bot:

Command	Response
temp	Temperature in °F
humidity	Relative humidity (%)
pressure	Barometric pressure (hPa)
gas	MQ-2 gas levels (ppm estimates)
status	Full sensor report
🛠 Hardware Required

ESP-12F (ESP8266)
AHT20 Temperature & Humidity Sensor (I²C)
BMP280 Pressure Sensor (I²C)
MQ-2 Gas Sensor (Analog)
Stable 3.3V power supply
WiFi connection

🔌 Wiring
I²C Sensors (AHT20 + BMP280)
ESP8266	Sensor
GPIO4 (D2)	SDA
GPIO5 (D1)	SCL
3.3V	VCC
GND	GND

BMP280 supports both 0x76 and 0x77 addresses (auto-detected).

MQ-2 Gas Sensor
ESP8266	MQ-2
A0	Analog Output
VCC	5V*
GND	GND

⚠️ Important:
Ensure the MQ-2 analog output is scaled to ≤ 1.0V for the ESP8266 ADC (use a voltage divider if needed).

Edit these values before uploading 
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID   "YOUR_CHAT_ID"

📚 Required Libraries

Install these via Arduino Library Manager:
ESP8266WiFi
WiFiClientSecure
UniversalTelegramBot
ArduinoJson
Wire
AHT20
Adafruit BMP280 Library

📊 Gas Sensor Notes

MQ-2 readings are approximate
R0 must be calibrated in clean air for best accuracy
PPM values are calculated using logarithmic curve fitting
Thresholds are defined but currently used for reporting only

📊 Gas Sensor Notes

MQ-2 readings are approximate
R0 must be calibrated in clean air for best accuracy
PPM values are calculated using logarithmic curve fitting
Thresholds are defined but currently used for reporting only
