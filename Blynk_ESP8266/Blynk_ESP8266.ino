/********************************************************************
 *                 Gumstix ESP8266 UV Sensor Board                  *
 *                          Blynk Demo                              *
 *                    (C) 2019 Gumstix, Inc.                        *
 ********************************************************************/
// Rename `blynk_config.h.dist` as `blynk_config.h` and add your WiFi credentials,
// Blynk auth token, etc.

#define BLYNK_PRINT Serial

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PolledTimeout.h>
#include <BlynkSimpleEsp8266.h>
#include "Adafruit_VEML6070.h"
#include <Adafruit_BME280.h>

#include "blynk_config.h"

#define SDA_PIN 2
#define SCL_PIN 14

Adafruit_BME280 bme;

// Container for barometer data
struct bme_data{
  float temp;  // Celsius
  float pres;  //
  uint32_t hum;
};

struct bme_data sensor_data;

// UV sensor returns the UV index
int uvindex;

Adafruit_VEML6070 uv = Adafruit_VEML6070();


void bme_get() {
  sensor_data.temp = bme.readTemperature(); 
  sensor_data.pres = bme.readPressure()/10.0; 
  sensor_data.hum = bme.readHumidity();
}


/*********Device Bridge**********/
// For the final demo there is also an RGB Matrix board that
// displays data from this and the ESP32 Gas Sensor board.
// This bridge allows the sensor boards to communicate with it

#ifdef BRIDGING
WidgetBridge bridge1(V1);

BLYNK_CONNECTED() {
  bridge1.setAuthToken(bridge_auth);
}
#endif // BRIDGING

void setup()
{
  // Debug console
  Serial.begin(9600);

  // Both sensors require I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initializing both sensors
  uv.begin(VEML6070_1_T);
  int status = bme.begin();
  if(!status){
    while(!Serial);
    Serial.println("BME connection failed!");
    while(1);
  }

  // Connect to Blynk server
  Blynk.begin(auth, ssid, pass, addr, 8080);
}

void loop()
{
// Using a timer to trigger measurements once a second.
  using periodic = esp8266::polledTimeout::periodic;
  static periodic nextPing(1000);
  if (nextPing){

// Collect sensor data and transmit to Blynk server
    uvindex = uv.readUV();
    bme_get();
    
    Blynk.virtualWrite(V0, uvindex);
    Blynk.virtualWrite(V1, sensor_data.temp);
    Blynk.virtualWrite(V2, sensor_data.pres / 100.0F);
    Blynk.virtualWrite(V3, sensor_data.hum);
    
#ifdef BRIDGING
    bridge1.virtualWrite(V0, uvindex);
    bridge1.virtualWrite(V1, sensor_data.temp);
    bridge1.virtualWrite(V2, sensor_data.pres / 100.0F);
    bridge1.virtualWrite(V3, sensor_data.hum);
#endif // BRIDGING

    Serial.print("UV Value:    "); Serial.println(uvindex);
    Serial.print("Temperature: "); Serial.println(sensor_data.temp);
    Serial.print("Pressure:    "); Serial.println(sensor_data.pres);
    Serial.print("Humidity:    "); Serial.println(sensor_data.hum);
  }
  
  Blynk.run();
}
