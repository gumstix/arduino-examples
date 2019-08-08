#include <gamma.h>
#include <Adafruit_NeoMatrix.h>
#include <BlynkSimpleMKR1000.h>
#include <Fonts/Picopixel.h>
#include <SPI.h>
#include <WiFi101.h>
#include "blynk_config.h"

#define RGB_PIN 4
#define BLYNK_PRINT Serial

uint8_t brightness = 30;

Adafruit_NeoMatrix matrix(16, 16, 2, 1, RGB_PIN,
                                  NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE + NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
                                  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                  NEO_GRB);
int fillColor = matrix.Color(85, 0, 128);
int textColor = matrix.Color(0,255,0);


int uvindex=0;
float temp=0.0;
float pressure=0.0;
float humidity=0.0;
uint8_t mode=0;

int gas_tvoc = 0;
int gas_eco2 = 0;
int gas_rawh2 = 0;
int gas_rawe = 0;
float gas_temp = 0.0;
float gas_pres = 0.0;
float gas_hum = 0.0;
float gas_gas = 0.0;
float gas_alt = 0.0;

uint8_t scroll = 0;
uint16_t scroll_counter = 0;

#define SCROLL_DELAY 100

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V17, brightness);
  Blynk.virtualWrite(V16, scroll);
}

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
  // put your setup code here, to run once:
  Serial.begin(9600);
  matrix.begin();
  matrix.setFont(&Picopixel);
  matrix.setTextWrap(false);
  matrix.setTextColor(textColor);
  matrix.setBrightness(30);
  matrix.fillScreen(0);
  matrix.show();
  Serial.println("Connecting to Blynk server:");
  Blynk.begin(auth, ssid, pass, IPAddress(192, 168,0,121), 8080);
  Serial.println("Connected");
  matrix.fillScreen(fillColor);
  matrix.show();
  
}

void loop() {
  matrix.fillScreen(fillColor);
  matrix.setTextColor(textColor);
  if(mode > 3) {
    matrix.fillRect(0, 0, 4, 4, 0);
    matrix.fillRect(0, 12, 4, 4, 0);
    matrix.fillRect(28, 0, 4, 4, 0);
    matrix.fillRect(28, 12, 4, 4, 0);
  }
  if(scroll) {
    scroll_counter++;
    matrix.drawLine(0, 0, (scroll_counter/3)-1, 0, matrix.Color(255, 0, 0));
    matrix.writePixel(scroll_counter/3, 0, matrix.Color(scroll_counter%3 * 255/3, 0, 0) | 
                                                       (fillColor & matrix.Color((3-scroll_counter%3) * 255/3, (3-scroll_counter%3) * 255/3, (3-scroll_counter%3) * 255/3)));
    if(scroll_counter > SCROLL_DELAY) {
      mode++;
      if(mode > 12)
        mode = 0;
      scroll_counter = 0;
    }
  }
  switch(mode) {
    case 0:
      matrix.setCursor(12, 5);
      matrix.print("UV:");
      matrix.setCursor(14, 13);
      matrix.print(uvindex);
      break;
    case 1:
      matrix.setCursor(10, 5);
      matrix.print("TMP:");
      matrix.setCursor(9, 13);
      matrix.print(temp);
      break;
    case 2:
      matrix.setCursor(10, 5);
      matrix.print("kPa:");
      matrix.setCursor(9, 13);
      matrix.print(pressure);
      break;
    case 3:
      matrix.setCursor(10, 5);
      matrix.print("HUM:");
      matrix.setCursor(9, 13);
      matrix.print(humidity);
      break;
    case 4:
      matrix.setCursor(8, 5);
      matrix.print("TVOC:");
      matrix.setCursor(8, 13);
      matrix.print(gas_tvoc);
      break;
    case 5:
      matrix.setCursor(8, 5);
      matrix.print("eCO2:");
      matrix.setCursor(10, 13);
      matrix.print(gas_eco2);
      break;
    case 6:
      matrix.setCursor(12, 5);
      matrix.print("H2:");
      matrix.setCursor(9, 13);
      matrix.print(gas_rawh2);
      break;
    case 7:
      matrix.setCursor(10, 5);
      matrix.print("Eth:");
      matrix.setCursor(6, 13);
      matrix.print(gas_rawe);
      break;
    case 8:
      matrix.setCursor(6, 5);
      matrix.print("TMP2:");
      matrix.setCursor(8, 13);
      matrix.print(gas_temp);
      break;
    case 9:
      matrix.setCursor(12, 5);
      matrix.print("Pa:");
      matrix.setCursor(0, 13);
      matrix.print(gas_pres);
      break;
    case 10:
      matrix.setCursor(8, 5);
      matrix.print("\%RH:");
      matrix.setCursor(8, 13);
      matrix.print(gas_hum);
      break;
    case 11:
      matrix.setCursor(0, 5);
      matrix.print("Gas Ohm:");
      matrix.setCursor(0, 13);
      matrix.print(gas_gas);
      break;
    case 12:
      matrix.setCursor(8, 5);
      matrix.print("Alt:");
      matrix.setCursor(8, 13);
      matrix.print(gas_alt);
      break;
    default:
      break;
  }
  
  matrix.show();
  delay(50);
  Blynk.run();
}
