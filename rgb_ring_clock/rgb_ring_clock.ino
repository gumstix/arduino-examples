

#include <Adafruit_ZeroTimer.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <WiFi101OTA.h>
#include <Adafruit_NeoPixel.h>
#include <SparkFun_Si7021_Breakout_Library.h>
#include <Wire.h>

#include "rgb_ring_clock.h"
#include "wifi_wpa.h"

#define NUM_PIXELS 16
#define D_PIN PB10
//#define D_PIN PA21
//#define D_PIN2 PB23
#define WIFI_TRIES 5
#define TIMEZONE -8 // PST (UTC -8)
// #define TIMEZONE -7 // PDT (UTC -7)
#define SEVENTY_YEARS 2208988800

#define INTENSITY 80

Weather sensor;

uint8_t the_time[] = {0,0,0,0,0};

bool check_time;

uint16_t interval = 0;

IPAddress timeServer(129,6,15,30);

uint32_t local_port = 2390;
uint32_t leds[NUM_PIXELS];

const uint8_t NTP_PACKET_SIZE = 48;

byte packetBuffer[NTP_PACKET_SIZE];

unsigned long raw_time = 0;

Adafruit_ZeroTimer zt3 = Adafruit_ZeroTimer(3);

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

void t_tick() {
  if(!check_time) {
    raw_time++;
    if(raw_time%300 == 0)
      check_time = true;
  }
}

Adafruit_NeoPixel rgb_array = Adafruit_NeoPixel(NUM_PIXELS, D_PIN, NEO_GRB + NEO_KHZ800);

int status = WL_IDLE_STATUS;

WiFiUDP Udp;

int last_sec;
int bright;
float temp = 0.0;
void setup() {
  Serial.begin(9600);
  check_time = false;
  rgb_array.begin();
  int8_t tries = 0;
  zt3.configure(TC_CLOCK_PRESCALER_DIV1024, TC_COUNTER_SIZE_16BIT, TC_WAVE_GENERATION_MATCH_FREQ);
  
  zt3.setCompare(0, 0xb75f);
  zt3.setCallback(true, TC_CALLBACK_CC_CHANNEL0, t_tick);
  while(status != WL_CONNECTED && tries < WIFI_TRIES) {
    status = WiFi.begin(WPA_SSID, WPA_PSK);
    tries++;
    for(int i=0; i<16; i++) {
      rgb_array.setPixelColor((15-i-1)%16, 0);
      rgb_array.setPixelColor((15-i)%16, rgb_array.Color(i*(i+1)/8 + 5, (i*(i+1)/4)%255, 10));
      rgb_array.show();
     delay(20);
    }
    
  }
  if(status == WL_CONNECTED) {
    Serial.print("WiFi connected.\n IP address = ");
    Serial.println(WiFi.localIP());
    Udp.begin(local_port);
    WiFiOTA.begin("Arduino", "password", InternalStorage);
  }
  last_sec = 0;
  bright = 1;
  sendNTPpacket(timeServer);
  zt3.enable(true);
  sensor.begin();
  sensor.heaterOff();
}


void loop() {
  // put your main code here, to run repeatedly:
  WiFiOTA.poll();
  uint16_t i;
  if(status == WL_CONNECTED) {
    if(check_time) {
      sendNTPpacket(timeServer);
      check_time = false;
    }
    int packetSize = Udp.parsePacket();
    
    if(packetSize) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      raw_time = highWord << 16 | lowWord;
    }
    
    temp = sensor.getTemp();
    uint8_t temp_led = temp - 20;
    uint8_t temp_dec = uint32_t((temp*10))%10;
    
    int current_sec = raw_time%60;
    if (current_sec != last_sec)
    {
      int top_bright =(60 - current_sec)/3;
      for(i=0; i<NUM_PIXELS-1; i++)
        leds[i] = 0;
      leds[NUM_PIXELS-1] = rgb_array.Color(top_bright, top_bright, top_bright);
   
      int sec_angle=current_sec*6;
      int sec_led=(2*sec_angle/45);
      int led_angle= (sec_led*45)/2;
      int balance=sec_angle-led_angle;
      sec_led = 15-sec_led;
      int sec_led_next;
      if(sec_led == 0)
        sec_led_next = 15;
      else
        sec_led_next = sec_led-1;
      leds[sec_led]|=rgb_array.Color((22-(balance%23))*(22-(balance%23))*(22-(balance%23))/INTENSITY,0,0);
      leds[sec_led_next]|=rgb_array.Color((balance%23)*(balance%23)*(balance%23)/INTENSITY,0,0);


      
      int current_sixth_min = (raw_time%3600)/10;
      sec_angle=current_sixth_min;
      sec_led=(2*sec_angle/45);
      led_angle= (sec_led*45)/2;
      balance=sec_angle-led_angle;
      sec_led = 15-sec_led;
      sec_led_next;
      if(sec_led == 0)
        sec_led_next = 15;
      else
        sec_led_next = sec_led-1;
      leds[sec_led]|=rgb_array.Color(0,(22-(balance%23))*(22-(balance%23))*(22-(balance%23))/INTENSITY,0);
      leds[sec_led_next]|=rgb_array.Color(0,(balance%23)*(balance%23)*(balance%23)/INTENSITY,0);

      
      int current_thirtieth_hr = ((raw_time+TIMEZONE*3600)%43200)/120;
      sec_angle=current_thirtieth_hr;
      sec_led=(2*sec_angle/45);
      led_angle= (sec_led*45)/2;
      balance=sec_angle-led_angle;
      sec_led = 15-sec_led;
      if(sec_led == 0)
        sec_led_next = 15;
      else
        sec_led_next = sec_led-1;
      leds[sec_led]|=rgb_array.Color(0,0,(22-(balance%23))*(22-(balance%23))*(22-(balance%23))/INTENSITY);
      leds[sec_led_next]|=rgb_array.Color(0,0,(balance%23)*(balance%23)*(balance%23)/INTENSITY);
      for(i=0;i<16;i++) {
        rgb_array.setPixelColor(i, leds[i]);
      }
      rgb_array.show();
      bright = (current_sec)%2;
      last_sec = current_sec;
      
    }
  }
  
}

unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

void convertTime(unsigned long raw_time) {
  unsigned long epoch = raw_time - SEVENTY_YEARS;
  int16_t hour = (epoch  % 86400L) / 3600;
  int16_t minute = (epoch  % 3600) / 60;
  int16_t sec = epoch % 60;
  hour += TIMEZONE;
  if(hour < 0)
    hour = 24 + hour;
  if(hour > 23)
    hour = hour - 24;

  the_time[0] = hour/10;
  the_time[1] = hour%10;
  the_time[2] = minute/10;
  the_time[3] = minute%10;
  the_time[4] = sec;   
}
