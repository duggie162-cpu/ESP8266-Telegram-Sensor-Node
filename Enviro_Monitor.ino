#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <AHT20.h>
#include <Adafruit_BMP280.h>
#include <math.h>

// =======================
// WIFI / TELEGRAM CONFIG
// =======================
const char* ssid = "aaaaaa";         //SSID
const char* password = "aaaaaaa";    // PW

#define BOT_TOKEN "aaaaaaaa"         //BOT ID telegram
#define CHAT_ID   "11111111"         //Chat ID                                   

// =======================
// SENSOR OBJECTS (GLOBAL)
// =======================
AHT20 aht;
Adafruit_BMP280 bmp;

// =======================
// MQ-2 GAS SENSOR
// =======================
#define MQ2_PIN A0
const float R0 = 10.0; // Sensor calibration in clean air

struct GasCurve {
  float m;
  float b;
  int thresholdPPM;
  String name;
};

GasCurve gases[] = {
  {-0.45, 0.22, 1000, "LPG"},
  {-0.48, 0.12, 300,  "CO"},
  {-0.38, 0.25, 500,  "Smoke"}
};
const int numGases = sizeof(gases)/sizeof(GasCurve);

// =======================
// TIME / TELEGRAM CLIENT
// =======================
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long lastTelegramCheck = 0;
const unsigned long telegramInterval = 3000;
int lastReportDay = -1;

// =======================
// FUNCTIONS
// =======================

// Convert ADC to Rs
float readRs() {
  int raw = analogRead(MQ2_PIN);
  float Vout = raw * (3.3 / 1023.0); // ESP8266 10-bit ADC
  float RL = 5.0;
  return RL * ((3.3 - Vout) / Vout);
}

// Rs -> PPM
float calculatePPM(float Rs, GasCurve gas) {
  float ratio = Rs / R0;
  return pow(10, (log10(ratio) - gas.b) / gas.m);
}

// Build sensor report
String buildReport() {
  String msg = "";

  float tC = aht.getTemperature();
  float tF = (tC * 9.0 / 5.0) + 32.0;
  float h = aht.getHumidity();
  float p = bmp.readPressure() / 100.0;
  float Rs = readRs();

  if (isnan(tC) || isnan(h)) return "❌ Sensor read error";

  msg += "📊 Daily Sensor Report\n";
  msg += "🌡 Temp: " + String(tF, 1) + " °F\n";
  msg += "💧 Humidity: " + String(h, 1) + " %\n";

  if (isnan(p)) msg += "🌬 Pressure: N/A\n";
  else msg += "🌬 Pressure: " + String(p, 1) + " hPa\n";

  msg += "🔥 Gas Levels:\n";
  for (int i = 0; i < numGases; i++) {
    float ppm = calculatePPM(Rs, gases[i]);
    msg += gases[i].name + ": " + String((int)ppm) + " ppm\n";
  }

  return msg;
}

// Check Telegram messages
void checkTelegram() {
  if (millis() - lastTelegramCheck < telegramInterval) return;
  lastTelegramCheck = millis();

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  if (numNewMessages == 0) return;

  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    text.toLowerCase();

    if (text == "temp") {
      float tC = aht.getTemperature();
      float tF = (tC * 9.0 / 5.0) + 32.0;
      bot.sendMessage(CHAT_ID, "🌡 Temp: " + String(tF,1) + " °F", "");
    }
    else if (text == "humidity")
      bot.sendMessage(CHAT_ID, "💧 Humidity: " + String(aht.getHumidity(),1) + " %", "");
    else if (text == "pressure") {
      float p = bmp.readPressure() / 100.0;
      if (isnan(p)) bot.sendMessage(CHAT_ID, "🌬 Pressure: N/A", "");
      else bot.sendMessage(CHAT_ID, "🌬 Pressure: " + String(p,1) + " hPa", "");
    }
    else if (text == "gas") {
      float Rs = readRs();
      String gasReport = "";
      for (int j = 0; j < numGases; j++) {
        float ppm = calculatePPM(Rs, gases[j]);
        gasReport += gases[j].name + ": " + String((int)ppm) + " ppm\n";
      }
      bot.sendMessage(CHAT_ID, "🔥 Gas Levels:\n" + gasReport, "");
    }
    else if (text == "status")
      bot.sendMessage(CHAT_ID, buildReport(), "");
  }
}

// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(4, 5); // SDA=D2(GPIO4), SCL=D1(GPIO5)
  delay(100);

  // BMP280 initialization check
  if (!bmp.begin(0x76) && !bmp.begin(0x77)) Serial.println("❌ BMP280 FAIL");
  else Serial.println("✅ BMP280 OK");

  if (!aht.begin()) Serial.println("❌ AHT20 not found");

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");

  client.setInsecure(); // Telegram HTTPS
  bot.sendMessage(CHAT_ID, "✅ ESP-12F Sensor Node Online", "");
}

// =======================
// LOOP
// =======================
void loop() {
  checkTelegram();

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  if (timeinfo) {
    if (timeinfo->tm_hour == 12 && timeinfo->tm_min == 0) {
      if (timeinfo->tm_yday != lastReportDay) {
        bot.sendMessage(CHAT_ID, buildReport(), "");
        lastReportDay = timeinfo->tm_yday;
      }
    }
  }

  delay(1000);
}
