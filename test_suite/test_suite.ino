//#define ROOMSENSE
//#define STRATA
//#define AEROCORE
#include <Wire.h>
#include "boards.h"

int i;  //Iterator
extern int gpio_pins[];
extern int pwm_pins[];
extern int ain_pins[];

char inbyte;
int inint;
int width;
int array_size;
int* pin_state;

#define GPIO 0
#define PWM 1
#define ADC 2
#define ALL 3
#define I2C 4
#define UART 5
#define BATCH 6
#define SPIBUS 7
int mode = GPIO;

#ifdef AEROCORE
extern SPIClass spi;
extern SPISettings spi_cfg;
#endif



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 
  while(!Serial);
  establishContact();
  array_size = sizeof(gpio_pins)/sizeof(int);
  pin_state = (int*)malloc(array_size*sizeof(int));
  for(i=0; i<array_size; i++)
  {
    pinMode(gpio_pins[i], OUTPUT);
    pin_state[i] = LOW;
    digitalWrite(gpio_pins[i], LOW);
  }
 
#ifndef AEROCORE  
#ifdef PIN_SERIAL1_RX
  Serial1.begin(9600);
  Serial.println("Serial1 enabled");
#endif //PIN_SERIAL1_RX
#else
  SPI.begin();
  Serial1.begin(9600);
  Serial3.begin(9600);
  Serial2.begin(9600);
  Serial7.begin(9600);
  Wire.setSCL(PIN_WIRE_SCL);
  Wire.setSDA(PIN_WIRE_SDA);
#endif
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(mode)
  {
    case GPIO:
      Serial.println("GPIO\n");
      Serial.println("\",\" for PWM");
      Serial.println("\".\" for ADC");
      Serial.println("\"/\" for ALL");
      Serial.println("\";\" for I2C");
      Serial.println("\"'\" for UART");
      Serial.println("\"[\" for BATCH");
      Serial.println("\"=\" for SPI");
      gpio_mode();
      break;
    case PWM:
      Serial.println("PWM");
      pwm_mode();
      break;
    case ADC:
      Serial.println("ADC");
      adc_mode();
      break;
    case ALL:
      Serial.println("ALL");
      all_mode();
      break;
    case I2C:
      Serial.println("I2C");
      i2c_mode();
      break;
    case UART:
      Serial.println("UART");
      uart_mode();
      break;
    case BATCH:
      Serial.println("BATCH");
      batch_mode();
      break;
    case SPIBUS:
#ifdef AEROCORE
      Serial.println("SPI");
      spi_mode();
      break;
#endif
    default:
      Serial.println("ERROR");
      mode = GPIO;
      break;
  }
 
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print('A');   // send a capital A
    delay(200);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(200);
  }
}

void gpio_mode()
{
  Serial.println("Enter pin to toggle:");
  while(Serial.available() == 0);
  inbyte = Serial.read();
  if(inbyte == ',')
  {
    mode = PWM;
    inint = 0;
  }
  if(inbyte == '=')
  {
    mode = SPIBUS;
    inint = 0;
  }
  if(inbyte == '.')
  {
    mode = ADC;
    inint = 0;
  }
  if(inbyte == '/')
  {
    mode = ALL;
    inint = 0;
  }
  if(inbyte == ';')
  {
    mode = I2C;
    inint = 0;
  }
  if(inbyte == '\'')
  {
    mode = UART;
    inint = 0;
  };
  if(inbyte == '[')
  {
    mode = BATCH;
    inint = 0;
  }
  else if(inbyte >= 'a' && inbyte <='z')
    inint = inbyte - 'a' + 10;
  else
    inint = inbyte - '0';
  
  if(inint > 0 and inint <= array_size)
  {
    Serial.println(gpio_pins[inint-1]);
    pin_state[inint-1] = !pin_state[inint-1];
    digitalWrite(gpio_pins[inint-1], pin_state[inint-1]);
  }
  while(Serial.available() > 0)
    inbyte = Serial.read();
}

void pwm_mode()
{
  Serial.println("Enter PWM:");
  while(Serial.available() <= 0);
  while(Serial.available() > 0)
    inint = Serial.parseInt();
  if(inint < sizeof(pwm_pins)/sizeof(int) + 1 and inint > 0)
  {
    Serial.print("Enter pulse width (0 - 9)for PWM");
    Serial.print(inint - 1);
    Serial.println(":");
    while(Serial.available() <= 0);
    while(Serial.available() > 0)
      width = Serial.parseInt();
    width = map(width, 0, 9, 0, 255);
    Serial.print("PWM");
    Serial.print(inint - 1);
    Serial.print(", width = ");
    Serial.println(width);
    analogWrite(pwm_pins[inint - 1], width);
  }
  else
    mode = GPIO;
}

void adc_mode()
{
  if(mode == ADC)
  {
    Serial.println("Press a key to scan analog pins: ");
    while(Serial.available() == 0);
    while(Serial.available() > 0)
    {
      inbyte = Serial.read();
      Serial.print(inbyte);
    }
  }
  Serial.println("");
  for(i=0; i<sizeof(ain_pins)/sizeof(int); i++)
  {
    inint = analogRead(ain_pins[i]);
    Serial.print("A");Serial.print(i);Serial.print(": ");Serial.println(inint);
  }
}

void all_mode()
{
  int pins_state= !pin_state[0];
  for(i=0; i<array_size; i++)
  {
    digitalWrite(i, pins_state);   
    pin_state[i] = pins_state;
  }
  Serial.print("All GPIOs set to ");Serial.println(pins_state);
  Serial.println("Press any key to toggle: ");
  while(Serial.available() == 0);
  while(Serial.available() > 0)
  {
    inbyte = Serial.read();
    if(inbyte == ';')
      mode = GPIO;
  }
}

void i2c_mode()
{
   if(mode == I2C)
   {
    Serial.println("Hit a key to send an I2C message");
    while(Serial.available() == 0);
    while(Serial.available() > 0)
    {
      inbyte = Serial.read();
      if(inbyte == ';')
        mode = GPIO;
    }
   }
    Wire.beginTransmission(4);
    Wire.write("Hello.");
    Wire.endTransmission();
    Wire.requestFrom(4, 2);
}

void uart_mode()
{
  if(mode == UART)
  {
    Serial.println("Hit a key to send a message over Serial1 (external UART):");
    while(Serial.available() == 0);
    while(Serial.available() > 0)
    {
      inbyte = Serial.read();
      if(inbyte == ';')
        mode = GPIO;
    }
  }
#ifndef AEROCORE
#ifdef PIN_SERIAL1_RX
  Serial1.println("Hello World from Serial1!");
#endif
#else
  Serial1.println("Serial1");
  Serial3.println("Serial3");
  Serial2.println("Serial2");
  Serial7.println("Serial7");
#endif
}

void spi_mode()
{
  if(mode == SPIBUS)
  {
    Serial.println("Hit a key to send a message over SPI");
    while(Serial.available() == 0);
    while(Serial.available() > 0)
    {
      inbyte = Serial.read();
      if(inbyte == ';')
        mode = GPIO;
    }
    spi.beginTransaction(spi_cfg);
    spi.transfer(22);
    spi.transfer(77);
    spi.endTransaction();
  }
}

void batch_mode()
{
  int j;
  Serial.println("Hit a key to perform batch operation:");
  while(Serial.available() == 0);
  while(Serial.available() > 0)
  {
    inbyte = Serial.read();
    if(inbyte == ';')
      mode = GPIO;
  }
  int set = HIGH;
  for(i=0;i<2;i++)
  {
    for(j=0; j<(sizeof(gpio_pins)/sizeof(int)); j++)
      digitalWrite(gpio_pins[j], set);
    delay(300);
    set = !set;
  }
  for(i=1; i<4; i++)
  {
    int pwm_width = i*(256/4) - 1;
//    for(j=0; j<(sizeof(pwm_pins)/sizeof(int)); j++)
//      analogWrite(pwm_pins[j], pwm_width);
    delay(100);
  }
  adc_mode();
  delay(100);
  i2c_mode();
  delay(100);
  uart_mode();
  delay(100);
  mode = GPIO;
}


