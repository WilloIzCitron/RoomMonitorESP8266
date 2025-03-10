#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ElegantOTA.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <DHT_U.h>
// #include <Adafruit_BME280.h> // Uncomment if using BME280
#include "MQ135.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define DHTPIN 14      // GPIO14 (D5 on Amica for a case)
#define DHTTYPE DHT22  // DHT 22 (AM2302)

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Adafruit_BME280 bme; // Uncomment if using BME280
MQ135 gasSensor = MQ135(0);
DHT_Unified dht(DHTPIN, DHTTYPE);

const char* ssid = "ssid";
const char* password = "pass";

AsyncWebServer server(80);

const int led = 13;
float temperature = 0.0;
int humidity = 0;
String airQ = "Unknown";
String status = "Unknown";
uint32_t delayMS;

const char html_part1[] PROGMEM =
  "<html>\
  <head>\
    <title>Home IoT: Room Quality</title>\
    <style>\
        body {\
            background-color: #121212;\
            color: #ffffff;\
            font-family: Arial, sans-serif;\
        }\
    </style>\
</head>\
<body>\
    <h1>Room Status</h1>\
    <div style=\"display: flex; flex-wrap: wrap; gap: 20px; justify-content: center; margin-top: 20px;\">\
        <div style=\"background-color: #222222; padding: 20px; border: 1px solid #444444; width: 200px; text-align: center;\">\
            <h2>Temperature</h2>\
            <p><span id=\"temperature\">";

const char html_part2[] PROGMEM =
  "</span>&deg;C</p>\
        </div>\
        <div style=\"background-color: #222222; padding: 20px; border: 1px solid #444444; width: 200px; text-align: center;\">\
            <h2>Air Quality</h2>\
            <p><span id=\"air-quality\">";

const char html_part3[] PROGMEM =
  "</span></p>\
        </div>\
        <div style=\"background-color: #222222; padding: 20px; border: 1px solid #444444; width: 200px; text-align: center;\">\
            <h2>Status</h2>\
            <p><span id=\"status\">";

const char html_part4[] PROGMEM =
  "</span></p>\
        </div>\
    </div>\
</body>\
</html>";

void airQStatus() {
  int ppm = round(gasSensor.getCorrectedPPM(temperature, humidity) / 100);
  if (ppm < 500) {
    airQ = "Healthy";
  } else if (ppm < 1000) {
    airQ = "Moderate";
  } else if (ppm < 1500) {
    airQ = "Unhealthy for Sensitive Group";
  } else if (ppm < 2000) {
    airQ = "Unhealthy";
  } else if (ppm < 3000) {
    airQ = "Very Unhealthy";
  } else {
    airQ = "Hazardous";
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  char tempStr[10];
  dtostrf(temperature, 6, 0, tempStr);  // Convert float to string with 2 decimal places
  String html = FPSTR(html_part1) + String(tempStr) + FPSTR(html_part2) + airQ + FPSTR(html_part3) + status + FPSTR(html_part4);
  request->send(200, "text/html", html);
}

void handleJSON(AsyncWebServerRequest *request){
  String json = 
  "{\
    \"temp\": \""+String(temperature)+"\",\
    \"airQuality\": \""+round(gasSensor.getCorrectedPPM(temperature, humidity) / 100)+"\",\
    \"humi\": \""+humidity+"\",\
    \"airQualityStatus\": \""+airQ+"\"\
  }";
  request->send(200, "application/json", json);
}

void updateDisplay() {
  char tempStr[10];
  dtostrf(temperature, 4, 0, tempStr);  // Convert float to string with 2 decimal places
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.drawRect(0, 0, 127, 10, 1);
  oled.drawRect(0, 10, 45, 50, 1);
  oled.drawRect(45, 10, 80, 25, 1);
  oled.drawRect(45, 32, 80, 25, 1);
  oled.setCursor(4, 14);
  oled.print(F("Temp."));
  oled.setCursor(4, 30);
  oled.print(String(tempStr));
  oled.drawCircle(35, 29, 1, 1);
  oled.setCursor(37, 30);
  oled.print(F("C"));
  oled.setCursor(47, 14);
  oled.print("Air Q: " + String(int(round(gasSensor.getCorrectedPPM(temperature, humidity) / 100))));
  oled.setCursor(47, 35);
  oled.print(status);
  oled.display();
  delay(delayMS);
}

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  Serial.begin(115200);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED ERROR");
  };
  oled.display();
  delay(5000);
  WiFi.begin(ssid, password);
  status = "Connecting...";
  updateDisplay();

  // if (!bme.begin(0x76)) { // Uncomment if using BME280
  //   status = "BME Error";
  //   Serial.println("Could not find a valid BME280 sensor, check wiring!");
  //   updateDisplay();
  //   while (1) {
  //     delay(100); // Prevent watchdog reset
  //   }
  // }

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("DHT Sensor detected with type: " + String(sensor.name) + " (" + String(sensor.version) + ")");
  delayMS = sensor.min_delay / 1000;

  Serial.print("Connecting to ");
  Serial.println(ssid);
  ElegantOTA.begin(&server, "admin", "admin");
  Serial.println("OTA Started");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  status = "Connected";
  updateDisplay();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api", HTTP_GET, handleJSON);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // temperature = bme.readTemperature(); // Uncomment if using BME280
  // humidity = bme.readHumidity();
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    status = "DHT Read Error";
  } else {
    temperature = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    status = "DHT Read Error";
  } else {
    humidity = event.relative_humidity;
  }
  airQStatus();
  ElegantOTA.loop();
  updateDisplay();
}
