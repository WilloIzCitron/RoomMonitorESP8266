# RoomMonitorESP8266
Room Monitoring (Air Quality, Temperature and Humidity) with ESP8266. the results are in 4 ways:
- OLED DIsplay
- WebServer
- JSON data
- Grafana (With PostgreSQL)

OTA (Over The Air) supported, can be flash to ESP8266 without USB/UART

## Flowchart
![Untitled Diagram drawio](https://github.com/user-attachments/assets/8077ecf8-2300-4c6d-9b98-1ad904a73aa7)

## Prerequites
- ESP8266
- Sensors (DHT22 & MQ-135)
- Server for Python code, Grafana and Postgres (256gb storage for database and grafana is recommended)
- OLED Display (optional)
- Python 3.19+
- Grafana
- PostgreSQL/MySQL/any SQL framework

## Setup
- need citation
- Download WebServerIoT.ino first
