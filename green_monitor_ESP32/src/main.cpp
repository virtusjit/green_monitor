#include <Arduino.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include <utils.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#define ONE_WIRE_BUS 2

#define WIFI_SSID "ASUS11_188"
#define WIFI_PASSWORD "vjqhjenth123"

#define MQTT_HOST IPAddress(192, 168, 1, 128)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

String temperatureString = "";     // переменная для хранения
                                   // данных о температуре
unsigned long previousMillis = 0;  // здесь хранится информация о том, 
                                   // когда в последний раз 
                                   // была опубликована температура
const long interval = 5000;        // интервал между публикациями 
                                   // данных от датчика

// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0xAB, 0xFF, 0x80, 0xA2, 0x21, 0x01, 0x48 };

void connectToWifi() {
Serial.println("Connecting to Wi-Fi...");
           //  "Подключаемся к WiFi..."
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
Serial.println("Connecting to MQTT...");
           //  "Подключаемся к MQTT..."
mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");  //  "Подключились к WiFi"
      Serial.println("IP address: ");  //  "IP-адрес: "
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
                 //  "WiFi-связь потеряна"
      // делаем так, чтобы ESP32
      // не переподключалась к MQTT
      // во время переподключения к WiFi:
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");                           //  "Подключились к MQTT."
    Serial.print("Session present: ");                              //  "Текущая сессия: "
    Serial.println(sessionPresent);
    // ESP32 подписывается на топик esp32/led
    //uint16_t packetIdSub = mqttClient.subscribe("esp32/led", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");               //  "Подписка при QoS 0, ID пакета: "
    //Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
             //  "Отключились от MQTT."
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
            //  "Публикация подтверждена."
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop() {
  /* trace_ds18b20_addr(oneWire);*/
  //Serial.println(read_tempC(sensors,sensor1));
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // сохраняем в переменную «previousMillis» время,
    // когда в последний раз были опубликованы новые данные:
    previousMillis = currentMillis;
    // новые данные о температуре:

    temperatureString = String(read_tempC(sensors,sensor1)); 

    Serial.println(temperatureString);

    uint16_t packetIdPub2 = mqttClient.publish("/green_monitor/temp/temp1", 2, true, temperatureString.c_str());
    Serial.print("Publishing on topic /green_monitor/temp/temp1 at QoS 2, packetId: ");  
             //  "Публикация в топик «/green_monitor/temp/temp1»
             //   при QoS 2, ID пакета: "
    Serial.println(packetIdPub2);
  }
}

