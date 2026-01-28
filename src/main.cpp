#include <Arduino.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>

// ===== OLED =====
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== WiFi / MQTT =====
#define WLAN_SSID       "Pasidaryk Pats"
#define WLAN_PASS       "pasidaryk-pats"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "doviliukas_1"
#define AIO_KEY         ""   // <-- TAVO ADAFRUIT IO KEY

// ===== DHT11 =====
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== RELAY =====
#define RELAY_PIN 26

// ===== MQTT =====
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish temperatureFeed =
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/termometras-v1");

Adafruit_MQTT_Publish humidityFeed =
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dregme-v1");

// ===== TIMING =====
unsigned long lastReadTime = 0;
const unsigned long readInterval = 5000;

unsigned long lastRelayTime = 0;

// ===== DATA =====
float temperature = 0;
float humidity = 0;

bool wifiOK = false;
bool relayState = false;

// ===== MQTT CONNECT =====
void MQTT_connect() {
  if (!mqtt.connected()) {
    int8_t ret = mqtt.connect();
    if (ret != 0) {
      mqtt.disconnect();
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  // OLED I2C
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ESP32 paleistas");
  display.display();

  // DHT
  dht.begin();

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // rėlė išjungta

  // WiFi (neblokuojantis)
  WiFi.begin(WLAN_SSID, WLAN_PASS);
}

// ================= LOOP =================
void loop() {

  // ===== WiFi būsena =====
  wifiOK = (WiFi.status() == WL_CONNECTED);

  // ===== DHT skaitymas kas 5 s =====
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temperature = t;
      humidity = h;

      Serial.print("Temp: ");
      Serial.print(temperature);
      Serial.print(" C | Hum: ");
      Serial.println(humidity);
    }
  }

  // ===== MQTT tik jei yra internetas =====
  if (wifiOK) {
    MQTT_connect();

    if (mqtt.connected()) {
      temperatureFeed.publish(temperature);
      humidityFeed.publish(humidity);
    }
  }

  // ===== RELAY kas 1 sek =====
  if (millis() - lastRelayTime >= 1000) {
    lastRelayTime = millis();
    relayState = !relayState;
    digitalWrite(RELAY_PIN, !relayState);
  }

  // ===== OLED =====
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Temperatura:");

  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print(temperature, 1);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Dregme: ");
  display.print(humidity, 1);
  display.print(" %");

  display.setCursor(90, 0);
  if (wifiOK) {
    display.print("WiFi");
  } else {
    display.print("OFF");
  }

  display.display();
}