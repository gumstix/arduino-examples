#include <bittable.h>
#include <FirmataDefines.h>
#include <FirmataParser.h>
#include <Firmata.h>
#include <FirmataMarshaller.h>
#include <FirmataConstants.h>
#include <Boards.h>


#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

// DMA NeoPixels work ONLY on SPECIFIC PINS.
// On Circuit Playground Express: 8, A2 and A7 (TX) are valid.
// On Feather M0, Arduino Zero, etc.: 5, 11, A5 and 23 (SPI MOSI).
// On GEMMA M0: pin 0.
// On Trinket M0: pin 4.
// On Metro M4: 3, 6, 8, 11, A3 and MOSI
#define PIN         23
#define NUM_PIXELS 256

Adafruit_NeoPixel_ZeroDMA rgb_array(NUM_PIXELS, PIN, NEO_GRB+NEO_KHZ800);


#include "rgb_array_clock.h"
#include "wifi_wpa.h"

#define WIFI_TRIES 5
#define TIMEZONE -8
#define SEVENTY_YEARS 2208988800
uint8_t the_time[] = {0,0,0,0,0};

bool check_time;

uint16_t interval = 0;

IPAddress timeServer(129,6,15,30);

uint32_t local_port = 2390;

const uint8_t NTP_PACKET_SIZE = 48;

byte packetBuffer[NTP_PACKET_SIZE];

unsigned long raw_time = 0;

const uint8_t* symbols[] = {
  symb_zero, symb_one, symb_two, symb_three, symb_four, symb_five, symb_six, symb_seven, 
  symb_eight,symb_nine, symb_a, symb_b, symb_c, symb_d, symb_e, symb_f };

int rgb_grid[256][3] = {
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
};

void t_tick() {
  if(!check_time) {
    raw_time++;
    if(raw_time%300 == 0)
      check_time = true;
  }
}

int status = WL_IDLE_STATUS;

WiFiUDP Udp;



void array_fill() {
  for(int i=0; i<NUM_PIXELS; i++) {
    rgb_array.setPixelColor(i, rgb_array.Color(rgb_grid[i][0], rgb_grid[i][1], rgb_grid[i][2]));
  }
}
int last_sec;
int bright;


void setup() {
  Serial.begin(9600);
  check_time = false;
  rgb_array.begin();
  rgb_array.show();
  int8_t tries = 0;
while(status != WL_CONNECTED && tries < WIFI_TRIES) {
    status = WiFi.begin(WPA_SSID, WPA_PSK);
    tries++;
    delay(200);
  }
  if(status == WL_CONNECTED) {
    Serial.print("WiFi connected.\n IP address = ");
    Serial.println(WiFi.localIP());
    Udp.begin(local_port);
  }
  last_sec = 0;
  bright = 1;
  sendNTPpacket(timeServer);
}

void loop() {
  // put your main code here, to run repeatedly:
  int i;
  if(status = WL_CONNECTED) {
    if(check_time) {
      sendNTPpacket(timeServer);
      check_time = false;
    }
    int packetSize = Udp.parsePacket();
    
    if(packetSize) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      convertTime(raw_time);
      Serial.print("The pre-ntp time is: ");
      Serial.print(the_time[0]);
      Serial.print(the_time[1]);
      Serial.print(":");
      Serial.print(the_time[2]);
      Serial.print(the_time[3]);
      Serial.print(":");
      Serial.println(the_time[4]);
      raw_time = highWord << 16 | lowWord;
      convertTime(raw_time);
      Serial.print("The time is:         ");
      Serial.print(the_time[0]);
      Serial.print(the_time[1]);
      Serial.print(":");
      Serial.print(the_time[2]);
      Serial.print(the_time[3]);
      Serial.print(":");
      Serial.println(the_time[4]);
    }
    int current_sec = raw_time%60;
    if (current_sec != last_sec)
    {
      convertTime(raw_time);
      int hr_led = ((the_time[0]*10 + the_time[1]) % 12) * (60/12);
      int min_led = (the_time[2]*10 + the_time[3]);
      int hr_offset = (min_led/15) - 2;
      hr_led = (hr_led+hr_offset)%60;
      if(current_sec > 0) {
        int top_bright =(60 - current_sec);
        rgb_array.setPixelColor(NUM_PIXELS-1, rgb_array.Color(top_bright, top_bright, top_bright));
        rgb_array.setPixelColor(seconds[59-current_sec], rgb_array.Color(bright*20, 5, bright*20));
      }
      else {
        for(i=0;i<NUM_PIXELS-1;i++)
          rgb_array.setPixelColor(i, rgb_array.Color(0,0,0));
        rgb_array.setPixelColor(NUM_PIXELS-1, rgb_array.Color(120,120,120));
        rgb_array.show();
      }
      rgb_array.setPixelColor(seconds[59-hr_led], rgb_array.Color(60,0,0));
      rgb_array.setPixelColor(seconds[59-min_led], rgb_array.Color(0,0,60));
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
  if(hour <= 0)
    hour = 24 + hour;
  if(hour > 23)
    hour = hour - 24;

  the_time[0] = hour/10;
  the_time[1] = hour%10;
  the_time[2] = minute/10;
  the_time[3] = minute%10;
  the_time[4] = sec;   
}
