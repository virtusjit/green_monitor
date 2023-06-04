#include <Arduino.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include <utils.h>
#include <HTTPClient.h>
#include <WiFi.h>
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include "main.h"

#define ONE_WIRE_BUS 2

#define WIFI_SSID "ASUS11_188"
#define WIFI_PASSWORD "vjqhjenth123"



#define uS_TO_S_FACTOR 1000000
                              
#define TIME_TO_SLEEP 5      

const char* serverName = "https://frukt24.ru/greenmon/devices";

TimerHandle_t wifiReconnectTimer;

String temperatureString = "";
String illuminationString = "";     // переменная для хранения
                                   // данных о температуре
unsigned long previousMillis = 0;  // здесь хранится информация о том, 
                                   // когда в последний раз 
                                   // была опубликована температура
const long interval = 60000;        // интервал между публикациями 
                                   // данных от датчика

// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0xAB, 0xFF, 0x80, 0xA2, 0x21, 0x01, 0x48 };
DeviceAddress sensor2 = { 0x28, 0x6, 0xF9, 0xCA, 0xA2, 0x21, 0x01, 0x39 }; //28 6 F9 CA A2 21 1 39
int publishStatus = 0;

void connectToWifi() {
Serial.println("Connecting to Wi-Fi...");
           //  "Подключаемся к WiFi..."
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}


void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");  //  "Подключились к WiFi"
      Serial.println("IP address: ");  //  "IP-адрес: "
      Serial.println(WiFi.localIP());
      //connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
                 //  "WiFi-связь потеряна"
      // делаем так, чтобы ESP32
      // во время переподключения к WiFi:
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}


void setup() {
  Serial.begin(115200);
  sensors.begin();

  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  //mqttClient.onConnect(onMqttConnect);
  

  connectToWifi();

}

void loop() {

  if (WiFi.status() == WL_CONNECTED) 
  {
      read_sensors();

      //+++++++ enable sleep timer
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      //  go to sleep
      esp_deep_sleep_start();

  }
  //================================================================

}

void read_sensors()
{

    HTTPClient http;
    
    String httpRequestData = "device=temp1&data=0.0";
    int httpResponseCode = 200;

    int retry = 5;
    
    temperatureString = String(read_tempC(sensors,sensor1));
    while (retry >0 and (temperatureString=="-127.00" or temperatureString=="85.00"))
    {
      temperatureString = String(read_tempC(sensors,sensor1));
      retry -=1; 
      delay(1000);
    }


    //Serial.println(httpResponseCode);
    if (temperatureString!="-127.00" and temperatureString!="85.00")
    {
        publishStatus = 0;
        Serial.println("temp1 "+temperatureString);

        http.begin(serverName);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpRequestData = "device=temp1&data=" + temperatureString;
        httpResponseCode = http.POST(httpRequestData);

    }

    retry = 5;
    temperatureString = String(read_tempC(sensors,sensor2)); 
    while (retry >0 and (temperatureString=="-127.00" or temperatureString=="85.00"))
    {
      temperatureString = String(read_tempC(sensors,sensor2));
      retry -=1;
      delay(1000);
    }

    if (temperatureString!="-127.00" and temperatureString!="85.00")
    {
      publishStatus = 0;
      Serial.println("temp2 "+temperatureString);

      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpRequestData = "device=temp2&data=" + temperatureString;
      httpResponseCode = http.POST(httpRequestData);
    }



    /*illuminationString = String(4095-analogRead(36));
    Serial.println(illuminationString);
    publishStatus = 0;
    uint16_t packetIdPub3 = mqttClient.publish("/green_monitor/sensors/illumination/illumination1", 2, true, illuminationString.c_str());
    Serial.print("Publishing on topic /green_monitor/sensors/illumination/illumination1 at QoS 2, packetId: ");  
             //  "Публикация в топик «/green_monitor/temp/temp1»
             //   при QoS 2, ID пакета: "*/


    
}



