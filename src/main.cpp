#include <Arduino.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>

// ----------- WiFi and Adafruit IO Setup ------------
#define WLAN_SSID       "HUAWEI P20 lite"        // your WiFi SSID
#define WLAN_PASS       "Doviliukasss1"      // your WiFi password

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "doviliukas_1"   // your Adafruit username
#define AIO_KEY         "aio_xvop37OBByhla94ZLLt2uFYJgcqL"  // your Adafruit IO key

// ----------- DHT11 Setup ------------
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----------- MQTT Setup ------------
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Feeds (create them in Adafruit IO first!)
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/termometras-v1");
Adafruit_MQTT_Publish humidityFeed    = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dregme-v1");

void MQTT_connect();

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Connecting to WiFi...");
  Serial.print (WLAN_SSID);
  Serial.print (WLAN_PASS);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
}

void loop() {
  MQTT_connect();

  float humi = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humi) || isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humi);
  Serial.print("%  Temperature: ");
  Serial.print(tempC);
  Serial.println("°C");

  // Publish to Adafruit IO
  if (!temperatureFeed.publish(tempC)) {
    Serial.println("Failed to publish temperature");
  } else {
    Serial.println("Temperature sent!");
  }

  if (!humidityFeed.publish(humi)) {
    Serial.println("Failed to publish humidity");
  } else {
    Serial.println("Humidity sent!");
  }

  delay(5000); // send data every 5 seconds
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1); // hang
    }
  }
  Serial.println("MQTT Connected!");
}