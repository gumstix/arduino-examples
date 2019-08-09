#include <gamma.h>
#include <Adafruit_NeoMatrix.h>
#include <BlynkSimpleMKR1000.h>
#include <Fonts/Picopixel.h>
#include <SPI.h>
#include <WiFi101.h>
#include "blynk_config.h"


#define RGB_PIN 4
#define BLYNK_PRINT Serial

/* Virtual pin map
 *  ESP8266:
 *  --------
 *     V0  - UV Index
 *     V1  - Temperature
 *     v2  - Pressure
 *     V3  - Humidity
 *  
 *  ESP32:
 *  ------
 *     V7  - TVOC
 *     V8  - eCO2
 *     V9  - Raw H2 (Hydrogen gas)
 *     V10 - Raw ethanol
 *     V11 - Temperature
 *     V12 - Pressure
 *     V13 - Humidity
 *     V14 - Gas Resistance
 *     V15 - Altitude
 *     
 *  Blynk App:
 *  ----------
 *     V4  - Mode switch (Progress to next datapoint
 *     V5  - Fill Color: RGB values for background
 *     V6  - Text Color
 *     V16 - Activate scroll (Changes mode after SCROLL_DELAY clicks)
 *     V17 - Brightness
 */

uint8_t brightness = 30;

Adafruit_NeoMatrix matrix(  16, 16, // 16x16 Matrix
                            2, 1, // 2 tiles in a row
                            RGB_PIN, // MKR1000 pin 4
                            NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE + // Start from top-left tile
                            NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE, // Start from top-left LED
                            NEO_GRB);

// Default starting colors
int fillColor = matrix.Color(85, 0, 128);
int textColor = matrix.Color(0,255,0);

// ESP8266 UV board variables
int uvindex=0;
float temp=0.0;
float pressure=0.0;
float humidity=0.0;

// ESP32 Gas board variables
int gas_tvoc = 0;
int gas_eco2 = 0;
int gas_rawh2 = 0;
int gas_rawe = 0;
float gas_temp = 0.0;
float gas_pres = 0.0;
float gas_hum = 0.0;
float gas_gas = 0.0;
float gas_alt = 0.0;

// Data display settings
uint8_t scroll = 0;
uint16_t scroll_counter = 0;
uint8_t mode=0;
uint8_t old_mode = 0;

// How long to wait to switch outputs when scrolling on
#define SCROLL_DELAY 32

// LCD widget on blynk app displays which mode is active
WidgetLCD lcd(V18);

// Make sure app knows current state at startup
BLYNK_CONNECTED() {
  Blynk.virtualWrite(V17, brightness);
  Blynk.virtualWrite(V16, scroll);
}

// BLYNK_WRITE is triggered when the server sends data to the device 
BLYNK_WRITE(V0) {
  uvindex = param.asInt();
}

BLYNK_WRITE(V1) {
  temp = param.asFloat();
}

BLYNK_WRITE(V2) {
  pressure = param.asFloat();
}

BLYNK_WRITE(V3) {
  humidity = param.asFloat();
}

BLYNK_WRITE(V4) {
  mode += param.asInt();
  if (mode > 12)
    mode = 0;
}

BLYNK_WRITE(V5) {
  fillColor = matrix.Color(param[0].asInt(), param[1].asInt(), param[2].asInt());
}

BLYNK_WRITE(V6) {
  textColor = matrix.Color(param[0].asInt(), param[1].asInt(), param[2].asInt());
}

BLYNK_WRITE(V7) {
  gas_tvoc = param.asInt();
}

BLYNK_WRITE(V8) {
  gas_eco2 = param.asInt();
}

BLYNK_WRITE(V9) {
  gas_rawh2 = param.asInt();
}

BLYNK_WRITE(V10) {
  gas_rawe = param.asInt();
}

BLYNK_WRITE(V11) {
  gas_temp = param.asFloat();
}

BLYNK_WRITE(V12) {
  gas_pres = param.asFloat();
}

BLYNK_WRITE(V13) {
  gas_hum = param.asFloat();
}

BLYNK_WRITE(V14) {
  gas_gas = param.asFloat();
}

BLYNK_WRITE(V15) {
  gas_alt = param.asFloat();
}

BLYNK_WRITE(V16) {
  scroll = param.asInt();
  scroll_counter = 0;
}

BLYNK_WRITE(V17) {
  brightness = param.asInt();
  matrix.setBrightness(brightness);
}


void setup() {
  Serial.begin(9600);
  matrix.begin();
  
  //Okay small font used
  matrix.setFont(&Picopixel);
  matrix.setTextWrap(false);
  matrix.setTextColor(textColor);
  matrix.setBrightness(30);

  // Wait for Blynk connection
  matrix.fillScreen(0);
  matrix.setCursor(2,10);
  matrix.print("Blynk?...");
  matrix.show();
  Serial.println("Connecting to Blynk server:");
  Blynk.begin(auth, ssid, pass, addr, 8080);
  Serial.println("Connected");
  matrix.fillScreen(fillColor);
  matrix.setCursor(8,10);
  
  // Yay, we're connected!!!
  matrix.print("BLYNK!");
  delay(1000);
  matrix.show();  
}

void loop() {
  matrix.fillScreen(fillColor);
  matrix.setTextColor(textColor);

  if(mode > 3) {
    // This is a visual cue for when you're seeing data from the ESP32
    matrix.fillRect(30, 0, 2, 2, 0);
    matrix.fillRect(30, 14, 2, 2, 0);
  }

  if(scroll) {
    // When time's up, switch to the next mode
    scroll_counter++;
    // Indicator line across the top
    matrix.drawLine(0, 0, scroll_counter, 0, matrix.Color(255, 0, 0));
    if(scroll_counter > SCROLL_DELAY) {
      mode++;
      if(mode > 12)
        mode = 0;
      scroll_counter = 0;
    }
  }
  
  // Before changing the text on the app's LCD widget, erase the existing data
  
  if(old_mode != mode) {
    lcd.clear();
    old_mode = mode;
  }
  if(mode < 4) // Which device are we looking at?
    lcd.print(0, 0, "ESP8266");
  else
    lcd.print(0, 0, "ESP32");
  switch(mode) {
    //  Master output switch:
    // Based on mode, display on RGB matrix the desired sensor value
    case 0:
      matrix.setCursor(12, 5);
      matrix.print("UV:");
      lcd.print(0,1, "UV Index");
      matrix.setCursor(14, 13);
      matrix.print(uvindex);
      break;
    case 1:
      matrix.setCursor(10, 5);
      matrix.print("TMP:");
      lcd.print(0,1, "Temperature (C)");
      matrix.setCursor(9, 13);
      matrix.print(temp);
      break;
    case 2:
      matrix.setCursor(10, 5);
      matrix.print("kPa:");
      lcd.print(0,1, "Baro Pressure (kPa)");
      matrix.setCursor(9, 13);
      matrix.print(pressure);
      break;
    case 3:
      matrix.setCursor(10, 5);
      matrix.print("HUM:");
      lcd.print(0,1, "Humidity (\%RH)");
      matrix.setCursor(9, 13);
      matrix.print(humidity);
      break;
    case 4:
      matrix.setCursor(8, 5);
      matrix.print("TVOC:");
      lcd.print(0,1, "TVOC (ppb)");
      matrix.setCursor(8, 13);
      matrix.print(gas_tvoc);
      break;
    case 5:
      matrix.setCursor(8, 5);
      matrix.print("eCO2:");
      lcd.print(0,1, "CO2 (ppm)");
      matrix.setCursor(10, 13);
      matrix.print(gas_eco2);
      break;
    case 6:
      matrix.setCursor(13, 5);
      matrix.print("H2:");
      lcd.print(0,1, "Raw H2");
      matrix.setCursor(9, 13);
      matrix.print(gas_rawh2);
      break;
    case 7:
      matrix.setCursor(3, 5);
      matrix.print("Ethanol:");
      lcd.print(0,1, "Raw Ethanol");
      matrix.setCursor(6, 13);
      matrix.print(gas_rawe);
      break;
    case 8:
      matrix.setCursor(6, 5);
      matrix.print("TMP2:");
      lcd.print(0,1, "Temperature (C)");
      matrix.setCursor(8, 13);
      matrix.print(gas_temp);
      break;
    case 9:
      matrix.setCursor(12, 5);
      matrix.print("Pa:");
      lcd.print(0,1, "Pressure (Pa)");
      matrix.setCursor(0, 13);
      matrix.print(gas_pres);
      break;
    case 10:
      matrix.setCursor(10, 5);
      matrix.print("\% RH:");
      lcd.print(0,1, "Humidity (\%RH)");
      matrix.setCursor(8, 13);
      matrix.print(gas_hum);
      break;
    case 11:
      matrix.setCursor(2, 5);
      matrix.print("Gas Ohm:");
      lcd.print(0,1, "Gas Resistance (Ohm)");
      matrix.setCursor(0, 13);
      matrix.print(gas_gas);
      break;
    case 12:
      matrix.setCursor(11, 5);
      matrix.print("Alt:");
      lcd.print(0,1, "Altitude (m)");
      matrix.setCursor(8, 13);
      matrix.print(gas_alt);
      break;
    default:
      break;
  }
  // And done!
  matrix.show();
  delay(50);
  Blynk.run();
}
