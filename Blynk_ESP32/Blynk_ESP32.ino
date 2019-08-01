
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

#define SEALEVELPRESSURE_HPA (1016.0)

Adafruit_BME680 bme; // I2C

struct sgp_data{
  int tvoc;
  int eco2;
  int raw_h2;
  int raw_e;
};
struct sgp_data sgp_d;

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

void get_sgp_data(){
  if(sgp.IAQmeasure()){
    sgp_d.tvoc = sgp.TVOC;
    sgp_d.eco2 = sgp.eCO2;
  }
  else{
    sgp_d.tvoc = -1;
    sgp_d.eco2 =-1;
  }
  if(sgp.IAQmeasureRaw()){
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
    bme_d.temp = bme.temperature;
    bme_d.pres = bme.pressure;
    bme_d.hum = bme.humidity;
    bme_d.gas = bme.gas_resistance;
  }
  else{
    bme_d.temp = -1;
    bme_d.pres = -1;
    bme_d.hum = -1;
    bme_d.gas = -1;
  }
  bme_d.alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
}
void setup() {
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("SGP30 test");

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

  Blynk.begin(auth, ssid, pass);
}


void loop() {
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
  Blynk.run();
  delay(1000);
}
