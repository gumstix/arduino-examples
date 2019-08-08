/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
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

struct bme_data{
  float temp;
  float pres;
  uint32_t hum;
};

struct bme_data sensor_data;
int uvindex;
Adafruit_VEML6070 uv = Adafruit_VEML6070();
WidgetBridge bridge1(V1);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

// Your WiFi credentials.
// Set password to "" for open networks.

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void bme_get() {
  sensor_data.temp = bme.readTemperature();
  sensor_data.pres = bme.readPressure()/10.0;
  sensor_data.hum = bme.readHumidity();
  
  float hum = getAbsoluteHumidity(sensor_data.temp, hum);
  Serial.println(hum);
}

BLYNK_CONNECTED() {
  bridge1.setAuthToken("4FstxCQ0xRtZO8qNAc0glNVniYVspzZY");
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  uv.begin(VEML6070_1_T);
  int status = bme.begin();
  if(!status){
    while(!Serial);
    Serial.println("BME connection failed!");
    while(1);
  }
  Blynk.begin(auth, ssid, pass, IPAddress(192, 168, 0, 121), 8080);
}

void loop()
{
  using periodic = esp8266::polledTimeout::periodic;
  static periodic nextPing(1000);
  if (nextPing)
  {
    uvindex = uv.readUV();
    bridge1.virtualWrite(V0, uvindex);
    bme_get();
    bridge1.virtualWrite(V1, sensor_data.temp);
    bridge1.virtualWrite(V2, sensor_data.pres / 100.0F);
    bridge1.virtualWrite(V3, sensor_data.hum);
    Serial.print("UV Value:    "); Serial.println(uvindex);
    Serial.print("Temperature: "); Serial.println(sensor_data.temp);
    Serial.print("Pressure:    "); Serial.println(sensor_data.pres);
    Serial.print("Humidity:    "); Serial.println(sensor_data.hum);
  }
  
  Blynk.run();
}
