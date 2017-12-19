/*  
 *   This is an example sketch for the Gumstix RoomSense ATSAMW25 USB dongle.
 *   It activates a power relay into wich a Christmas Tree is plugged.
 *   The tree's lights turn on when the room is occupied and deactivates
 *   after a variable interval (in minutes) of inactivity elapses.
 *   
 *   Also delivers relative humidity, temperature and barometric pressure to
 *   the Cayenne application.
 *   
 *   Circuit:
 *   Pin 10 connected to +ve terminal of relay
 *   GND connected to -ve terminal of relay
 *   
 *   Created: 11 December 2017
 *   by Keith Lee
 */



#define RELAY_PIN 10
//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#define TX_INTERVAL 300
#define PIR_DOUT 0
#define BRIGHT_LED 9


#include <CayenneMQTTMKR1000.h>
#include "settings.h"
#include <SparkFun_Si7021_Breakout_Library.h>
#include <Wire.h>
#include <MS5611.h>
//#include <WiFi101OTA.h>

int idleTime = 0;
bool wasActive = false;
bool relayOn = false;
bool updateIdleMinutes = true;

float humidity = 0;
float temp = 0;
float pressure = 0;
int idleMinutes = 30;
int millisStart;
int millisInter;
int millisLastTx;
Weather sensor;
MS5611 Psensor(&Wire);

int pirStatus;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIR_DOUT, INPUT);
  pinMode(BRIGHT_LED, OUTPUT);
  digitalWrite(BRIGHT_LED, HIGH);
  digitalWrite(RELAY_PIN, LOW);
  
  Serial.begin(9600);
  Psensor.connect();
  sensor.begin();

  Cayenne.begin(mqttUser, mqttPass, mqttID, ssid, pass);
  millisStart = millis();
  millisLastTx = millisStart;

//  WiFiOTA.begin("OfficeTree", otaPass, InternalStorage);
}

void loop() {
pirStatus = digitalRead(PIR_DOUT);
  if(pirStatus && !wasActive)
  {
    if(!relayOn)
    {
      digitalWrite(RELAY_PIN, HIGH);
      relayOn = true;  
      idleTime = 0;
      Serial.print("Relay going active\n");
    }
    wasActive = true;
    
  }
  else if(!pirStatus)
  {
    if(wasActive)
    {
      wasActive = false;
      millisStart = millis();
      Serial.print("Occupant has left\n");
    }
    else
    {
      
      millisInter = millis();
      if((millisInter%2) || (millisInter%2000))
      {
        digitalWrite(BRIGHT_LED, !(digitalRead(BRIGHT_LED)));
      }
      if(millisInter - millisLastTx >= TX_INTERVAL)
      {
        txData();
        // Serial.print("Transmitted to Cayenne\n");
        millisLastTx = millisInter;
      }
      if(millisInter - millisStart > 60000)
      {
        idleTime++;
        millisStart = millisInter;
        Serial.print("Tick\n");
      }
    }
    if(idleTime >= idleMinutes && relayOn)
    {
      digitalWrite(RELAY_PIN, LOW);
      relayOn = false;
      Serial.print("Time expired.  Relay off\n");
    }
  }
//  WiFiOTA.poll();
  Cayenne.loop();
}

void txData()
{
  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  temp = sensor.getTemp();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()

  Psensor.ReadProm();
  Psensor.Readout();
  pressure = Psensor.GetPres()/100.0;
  Cayenne.celsiusWrite(1, temp);
  Cayenne.virtualWrite(2, humidity, "rel_hum","p");
  Cayenne.virtualWrite(3, pressure, "bp", "pa");
  Cayenne.virtualWrite(5, relayOn, "prox", "bool");
  Cayenne.virtualWrite(6, idleTime);
  Cayenne.virtualWrite(7, relayOn);
  if(updateIdleMinutes)
  {
    Cayenne.virtualWrite(4, idleMinutes);
    updateIdleMinutes = false;
  }

}

CAYENNE_IN(4)
{
  idleMinutes = getValue.asInt();
  updateIdleMinutes = true;
  Serial.print("Recived new idle timeout\n");
}
CAYENNE_IN(7)
{
  relayOn = !(getValue.asInt() == 0);
  digitalWrite(RELAY_PIN, relayOn);
  idleTime = 0;
  wasActive = true;
  Serial.print("Manually toggled the tree");
}

