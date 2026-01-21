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

// ----------- WiFi and Adafruit IO Setup ------------
#define WLAN_SSID       "Pasidaryk Pats"
#define WLAN_PASS       "pasidaryk-pats"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "doviliukas_1"   // your Adafruit username
#define AIO_KEY         ""  // your Adafruit IO key

// ----------- DHT11 Setup ------------
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----------- MQTT Setup ------------
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish temperatureFeed =
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/termometras-v1");

Adafruit_MQTT_Publish humidityFeed =
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dregme-v1");

void MQTT_connect();

void setup() {
  Serial.begin(9600);

  // ===== OLED init =====
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED nerastas");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ESP32 paleistas");
  display.display();

  // ===== WiFi =====
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  display.println("WiFi OK");
  display.display();

  dht.begin();
}

void loop() {
  MQTT_connect();

  float humi = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humi) || isnan(tempC)) {
    Serial.println("DHT klaida");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("DHT klaida!");
    display.display();
    delay(2000);
    return;
  }

  // ===== OLED OUTPUT =====
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Temperatura:");
  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print(tempC, 1);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Dregme: ");
  display.print(humi, 1);
  display.print(" %");

  display.display();

  // ===== Serial =====
  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.print(" C | Humidity: ");
  Serial.print(humi);
  Serial.println(" %");

  // ===== MQTT Publish =====
  temperatureFeed.publish(tempC);
  humidityFeed.publish(humi);

  delay(5000);
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) return;

  Serial.print("Connecting to MQTT...");
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT connected!");
}