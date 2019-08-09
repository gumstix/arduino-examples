/********************************************************************
 *                 Gumstix ESP32 Gas Sensor Board                   *
 *                          Blynk Demo                              *
 *                    (C) 2019 Gumstix, Inc.                        *
 ********************************************************************/
// Rename blynk_config.h.dist as blynk_config.h and add your WiFi credentials,
// Blynk auth token, etc.
 
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "blynk_config.h"

#define SDA_PIN 18
#define SCL_PIN 19

Adafruit_SGP30 sgp;

//Pressure at sea level is ~101.3 kPa
#define SEALEVELPRESSURE_HPA (1013.0)

Adafruit_BME680 bme; // I2C

// Container for SGP30 gas sensor data
struct sgp_data{
  int tvoc;
  int eco2;
  int raw_h2;
  int raw_e;
};
struct sgp_data sgp_d;

// Container for BME680 barmometer data
struct bme_data{
  float temp;
  float pres;
  float hum;
  float gas;
  float alt;
};
struct bme_data bme_d;

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}


// Get data from sensors
void get_sgp_data(){
  if(sgp.IAQmeasure()){
    sgp_d.tvoc = sgp.TVOC; // parts per billion
    sgp_d.eco2 = sgp.eCO2; // parts per millon 
  }
  else{
    sgp_d.tvoc = -1;
    sgp_d.eco2 =-1;
  }
  if(sgp.IAQmeasureRaw()){
  // This is raw sensor data.  It has no units.
    sgp_d.raw_h2 = sgp.rawH2;    
    sgp_d.raw_e = sgp.rawEthanol;
  }
  else{
    sgp_d.raw_h2 = -1;
    sgp_d.raw_e = -1;
  }
}

void get_bme_data(){
  if(bme.performReading()){
    bme_d.temp = bme.temperature; // Celsius
    bme_d.pres = bme.pressure; // Pascals
    bme_d.hum = bme.humidity; // % relative humidity
    bme_d.gas = bme.gas_resistance; // Ohms
  }
  else{
    bme_d.temp = -1;
    bme_d.pres = -1;
    bme_d.hum = -1;
    bme_d.gas = -1;
  }
  bme_d.alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
}


/*********Device Bridge**********/
// For the final demo there is also an RGB Matrix board that
// displays data from this and the ESP8266 UV Sensor board
// This bridge allows the sensor boards to communicate with it

#ifdef BRIDGING
WidgetBridge bridge1(V8);
BLYNK_CONNECTED() {
  bridge1.setAuthToken(bridge_auth);
}
#endif // BRIDGING

void setup() {
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);

// Verify sensors are working
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  get_bme_data();

  sgp.setHumidity(getAbsoluteHumidity(bme_d.temp, bme_d.pres));

// Connect to the Blynk server
  Blynk.begin(auth, ssid, pass, addr, 8080);
}


void loop() {
  // The loop simply collects the sensor data and sends it to
  // the Blynk server for consumption.
  
  get_sgp_data();
  get_bme_data();
  Serial.print("Altitude: ");Serial.println(bme_d.alt);
  Serial.print("Temperature: "); Serial.println(bme_d.temp);
  Blynk.virtualWrite(0, sgp_d.tvoc);
  Blynk.virtualWrite(1, sgp_d.eco2);
  Blynk.virtualWrite(2, sgp_d.raw_h2);
  Blynk.virtualWrite(3, sgp_d.raw_e);
  Blynk.virtualWrite(4, bme_d.alt);
  Blynk.virtualWrite(5, bme_d.pres);
  Blynk.virtualWrite(6, bme_d.hum);
  Blynk.virtualWrite(7, bme_d.gas);

// If you're using a bridge to an RGB matrix board, or some
// other remote consumer, use the following to transmit.
#ifdef BRIDGING
  bridge1.virtualWrite(7, sgp_d.tvoc);
  bridge1.virtualWrite(8, sgp_d.eco2);
  bridge1.virtualWrite(9, sgp_d.raw_h2);
  bridge1.virtualWrite(10, sgp_d.raw_e);
  bridge1.virtualWrite(11, bme_d.temp);
  bridge1.virtualWrite(12, bme_d.pres);
  bridge1.virtualWrite(13, bme_d.hum);
  bridge1.virtualWrite(14, bme_d.gas);
  bridge1.virtualWrite(15, bme_d.alt);
#endif //BRIDGING
  Blynk.run();
  delay(1000);
}
