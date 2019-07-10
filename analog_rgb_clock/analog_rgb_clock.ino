#include <Adafruit_ZeroTimer.h>

#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <tgmath.h>
#include <math.h>

#include "analog_rgb_clock.h"
#include "wifi_wpa.h"

#define NUM_PIXELS 256
// #define D_PIN PB10
#define D_PIN PA21
#define WIFI_TRIES 5
#define TIMEZONE -8
#define SEVENTY_YEARS 2208988800


int p_index(int* pixel) {
  int x, y, idx;
  if(pixel[0] >= 0)
    x = ceil(axis[0]);
  else
    x = floor(axis[0]);
  x += pixel[0];
  
  if(pixel[1] >= 0)
    y = ceil(axis[1]);
  else
    y = floor(axis[1]);
  y += pixel[1];

//  Serial.print("pixel index = (");
//  Serial.print(x);
//  Serial.print(",");
//  Serial.print(y);
//  Serial.println(")");
  if(x < ARRAY_W && y < ARRAY_H)
    idx = y * int(ARRAY_W) + x;
  return idx;
}
byte get_q(float theta) {
  byte q = 0; 
  if(theta < 90)
    q = 0;
  else if(theta < 180)
    q = 1;
  else if(theta < 270)
    q = 2;
  else
    q = 3;
//    Serial.println(q);
  return q;
}

void quadrantize(float theta, int* pixel) {
  byte q = get_q(theta);
  int tmp;
  switch(q) {
    case 0:
      if(pixel[0] < 0) pixel[0] = -1 * pixel[0];
      if(pixel[1] < 0) pixel[1] = -1 * pixel[1];
      break;
    case 1:
      tmp = pixel[0];
      pixel[0] = pixel[1];
      pixel[1] = pixel[0];
      if(pixel[0] < 0) pixel[0] = -1 * pixel[0];
      if(pixel[1] > 0) pixel[1] = -1 * pixel[1];
      break;
    case 2:
      if(pixel[0] > 0) pixel[0] = -1 * pixel[0];
      if(pixel[1] > 0) pixel[1] = -1 * pixel[1];
      break;
    case 3:
      tmp = pixel[0];
      pixel[0] = pixel[1];
      pixel[1] = pixel[0];
      if(pixel[0] > 0) pixel[0] = -1 * pixel[0];
      if(pixel[1] < 0) pixel[1] = -1 * pixel[1];
      break;
    default: 
      break;
  }
  Serial.print("("); Serial.print(pixel[0]); Serial.print(","); Serial.print(pixel[1]); Serial.println(");");
}


void console_home_location() {
  Serial.write(27);   
  Serial.print("[2J");    
  Serial.write(27);
  Serial.print("[H");
}

int pixel_pair[2][2];

void est_pixels(float x, float y) {
  pixel_pair[0][0] = (int)floor(x);
  pixel_pair[1][0] = (int)ceil(x);

  pixel_pair[0][1] = (int)floor(y);
  pixel_pair[1][1] = (int)ceil(y);
}
void calculate_arm(float theta, float radius, int* pixels) {
  Serial.println(theta);
  
  int idx;
  int x = 0;
  int y = 0;
  float x_point[2];
  float y_point[2];
  byte q = get_q(theta);
  float phi = theta - ((float)q * 90.0);
  float hypot = 0.0;
  float tangent = tan(phi);
  if(theta == 90.0 || theta == 270.0) {
    for(x = 0; x < radius; x++) {
      est_pixels(x, 0);
      quadrantize(theta, pixel_pair[0]);
        pixels[p_index(pixel_pair[0])] = 1;
        pixels[p_index(pixel_pair[1])] = 1;
        
    }
  }
  else if(theta == 0 || theta == 180 || theta == 360) {
    for(x = 0; x < radius; x++) {
      est_pixels(0, x);
      quadrantize(theta, pixel_pair[0]);
      pixels[p_index(pixel_pair[0])] = 1;
      pixels[p_index(pixel_pair[1])] = 1;
    }
  }
  
  else while(hypot <= radius) {
    x_point[0] = (float)x;
    x_point[1] = tangent * (float)x;
    if(y == 0)
      y_point[0] = 0;
    else
      y_point[0] = tangent / (float)y;
    y_point[1] = (float)y;
    hypot = sqrt(y_point[0]*y_point[0] + x_point[1]*x_point[1]);
    est_pixels(x_point[0], x_point[1]);
    quadrantize(theta, pixel_pair[0]);
    quadrantize(theta, pixel_pair[1]);
    if(sqrt(x_point[1]*x_point[1] + x_point[0]*x_point[0]) < radius) {
      pixels[p_index(pixel_pair[0])] = 1;
      pixels[p_index(pixel_pair[1])] = 1;
    }
    est_pixels(y_point[0], y_point[1]);
    quadrantize(theta, pixel_pair[0]);
    quadrantize(theta, pixel_pair[1]);
    if(sqrt(y_point[0]*y_point[0] + y_point[1]*y_point[1]) < radius) {
      pixels[p_index(pixel_pair[0])] = 1;
      pixels[p_index(pixel_pair[1])] = 1;
    }
    y++;
    x++;
  }
  
  
}



//uint8_t the_time[] = {0,0,0,0,0};
//
//bool check_time;
//
//uint16_t interval = 0;
//
//IPAddress timeServer(129,6,15,30);
//
//uint32_t local_port = 2390;
//
//const uint8_t NTP_PACKET_SIZE = 48;
//
//byte packetBuffer[NTP_PACKET_SIZE];
//
//unsigned long raw_time = 0;

int pixels[(int)(ARRAY_W*ARRAY_H)];
//
//const uint8_t* symbols[] = {
//  symb_zero, symb_one, symb_two, symb_three, symb_four, symb_five, symb_six, symb_seven, 
//  symb_eight,symb_nine, symb_a, symb_b, symb_c, symb_d, symb_e, symb_f };
//
//int rgb_grid[256][3] = {
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//  {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
//};
//
//Adafruit_ZeroTimer zt3 = Adafruit_ZeroTimer(3);
//
//void TC3_Handler() {
//  Adafruit_ZeroTimer::timerHandler(3);
//}
//
//void t_tick() {
//  if(!check_time) {
//    raw_time++;
//    if(raw_time%300 == 0)
//      check_time = true;
//  }
//}
//
//Adafruit_NeoPixel rgb_array = Adafruit_NeoPixel(NUM_PIXELS, D_PIN, NEO_GRB + NEO_KHZ800);
//
//int status = WL_IDLE_STATUS;
//
//WiFiUDP Udp;
//
//
//
//void array_fill() {
//  for(int i=0; i<NUM_PIXELS; i++) {
//    rgb_array.setPixelColor(i, rgb_array.Color(rgb_grid[i][0], rgb_grid[i][1], rgb_grid[i][2]));
//  }
//}
//int last_sec;
//int bright;
//

float theta;
void setup() {
  Serial.begin(9600);
  
  theta = 0.1 ;
//  check_time = false;
//  rgb_array.begin();
//  rgb_array.show();
//  int8_t tries = 0;
//
//  zt3.configure(TC_CLOCK_PRESCALER_DIV1024, TC_COUNTER_SIZE_16BIT, TC_WAVE_GENERATION_MATCH_FREQ);
//  
//  zt3.setCompare(0, 0xb75f);
//  zt3.setCallback(true, TC_CALLBACK_CC_CHANNEL0, t_tick);
//  while(status != WL_CONNECTED && tries < WIFI_TRIES) {
//    status = WiFi.begin(WPA_SSID, WPA_PSK);
//    tries++;
//    delay(200);
//  }
//  if(status == WL_CONNECTED) {
//    Serial.print("WiFi connected.\n IP address = ");
//    Serial.println(WiFi.localIP());
//    Udp.begin(local_port);
//  }
//  last_sec = 0;
//  bright = 1;
//  sendNTPpacket(timeServer);
//  zt3.enable(true);
  while(!Serial);
}

void loop() {
  int x, y, index;
  for(int i = 0; i < ARRAY_W*ARRAY_H; i++)
    pixels[i] = 0;
  calculate_arm(theta, 6.0, pixels);
//  Serial.print(".");
  for(x = 0; x < ARRAY_W; x++) {
    for(y = 0; y < ARRAY_H; y++) {
      index = y * ARRAY_W + x;
      Serial.print(pixels[index]);
      Serial.print("."); 
    }
    Serial.print("\n.");
   
  }
  
  theta += 6.0;
  if(theta >= 360.0)
    theta = 0.0;
  delay(1000);
  // put your main code here, to run repeatedly:
//  int i;
//  if(status = WL_CONNECTED) {
//    if(check_time) {
//      sendNTPpacket(timeServer);
//      check_time = false;
//    }
//    int packetSize = Udp.parsePacket();
//    
//    if(packetSize) {
//      Udp.read(packetBuffer, NTP_PACKET_SIZE);
//      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
//      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
//      convertTime(raw_time);
//      Serial.print("The pre-ntp time is: ");
//      Serial.print(the_time[0]);
//      Serial.print(the_time[1]);
//      Serial.print(":");
//      Serial.print(the_time[2]);
//      Serial.print(the_time[3]);
//      Serial.print(":");
//      Serial.println(the_time[4]);
//      raw_time = highWord << 16 | lowWord;
//      convertTime(raw_time);
//      Serial.print("The time is:         ");
//      Serial.print(the_time[0]);
//      Serial.print(the_time[1]);
//      Serial.print(":");
//      Serial.print(the_time[2]);
//      Serial.print(the_time[3]);
//      Serial.print(":");
//      Serial.println(the_time[4]);
//    }
//    int current_sec = raw_time%60;
//    if (current_sec != last_sec)
//    {
//      convertTime(raw_time);
//      int hr_led = ((the_time[0]*10 + the_time[1]) % 12) * (60/12);
//      int min_led = (the_time[2]*10 + the_time[3]);
//      int hr_offset = (min_led/15) - 2;
//      hr_led = (hr_led+hr_offset)%60;
//      if(current_sec > 0) {
//        int top_bright =(60 - current_sec);
//        rgb_array.setPixelColor(NUM_PIXELS-1, rgb_array.Color(top_bright, top_bright, top_bright));
//        rgb_array.setPixelColor(seconds[59-current_sec], rgb_array.Color(bright*20, 5, bright*20));
//      }
//      else {
//        for(i=0;i<NUM_PIXELS-1;i++)
//          rgb_array.setPixelColor(i, rgb_array.Color(0,0,0));
//        rgb_array.setPixelColor(NUM_PIXELS-1, rgb_array.Color(120,120,120));
//        rgb_array.show();
//      }
//      rgb_array.setPixelColor(seconds[59-hr_led], rgb_array.Color(60,0,0));
//      rgb_array.setPixelColor(seconds[59-min_led], rgb_array.Color(0,0,60));
//      rgb_array.show();
//      bright = (current_sec)%2;
//      last_sec = current_sec;
//    }
//  }

  
}
//
//unsigned long sendNTPpacket(IPAddress& address)
//{
//  //Serial.println("1");
//  // set all bytes in the buffer to 0
//  memset(packetBuffer, 0, NTP_PACKET_SIZE);
//  // Initialize values needed to form NTP request
//  // (see URL above for details on the packets)
//  //Serial.println("2");
//  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
//  packetBuffer[1] = 0;     // Stratum, or type of clock
//  packetBuffer[2] = 6;     // Polling Interval
//  packetBuffer[3] = 0xEC;  // Peer Clock Precision
//  // 8 bytes of zero for Root Delay & Root Dispersion
//  packetBuffer[12]  = 49;
//  packetBuffer[13]  = 0x4E;
//  packetBuffer[14]  = 49;
//  packetBuffer[15]  = 52;
//
//  //Serial.println("3");
//
//  // all NTP fields have been given values, now
//  // you can send a packet requesting a timestamp:
//  Udp.beginPacket(address, 123); //NTP requests are to port 123
//  //Serial.println("4");
//  Udp.write(packetBuffer, NTP_PACKET_SIZE);
//  //Serial.println("5");
//  Udp.endPacket();
//  //Serial.println("6");
//}
//
//void convertTime(unsigned long raw_time) {
//  unsigned long epoch = raw_time - SEVENTY_YEARS;
//  int16_t hour = (epoch  % 86400L) / 3600;
//  int16_t minute = (epoch  % 3600) / 60;
//  int16_t sec = epoch % 60;
//  hour += TIMEZONE;
//  if(hour <= 0)
//    hour = 24 + hour;
//  if(hour > 23)
//    hour = hour - 24;
//
//  the_time[0] = hour/10;
//  the_time[1] = hour%10;
//  the_time[2] = minute/10;
//  the_time[3] = minute%10;
//  the_time[4] = sec;   
//}

