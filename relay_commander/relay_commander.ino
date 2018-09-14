#include <FlashAsEEPROM.h>
#include <FlashStorage.h>

#define MAX_WIFI_ATTEMPTS 5
#define DEBUG

#include <SPI.h>
#include <WiFi101.h>

const int num_relays = 10;

typedef enum lockout_value_t {none, console, network} lockout_value;

lockout_value lockout;

typedef struct relay_state_t {
  int current;
  int next; 
} relay_state;

typedef struct wpa_data_t {
  char ssid[100];
  char psk[100];
} wpa_data;

typedef struct debounce_state_t {
  int button_state;
  int last_button_state;
  unsigned long last_debounce_time;
  int value;
} debounce_state;

const unsigned long debounce_delay = 50;


relay_state relay_states[num_relays];
debounce_state buttons[num_relays];

wpa_data credentials;
int wifi_status = WL_IDLE_STATUS;

FlashStorage(wpa_credentials, wpa_data);

int relay_clockwise[] =   {
                            PB3,
                            PA2,
                            PA4,
                            PA6,
                            PA8,
                            PA10,
                            PA16,
                            PA19,
                            PA21,
                            PA23
                          };
int button_clockwise[] =  {
                            PB10,
                            PA3,
                            PA5,
                            PA7,
                            PA9,
                            PA11,
                            PA17,
                            PA20,
                            PA22,
                            PB2
                          };

int seq_index;

void attempt_login() {
  int attempts = 0;
  while(wifi_status != WL_CONNECTED && attempts < MAX_WIFI_ATTEMPTS) {
    wifi_status = WiFi.begin(credentials.ssid, credentials.psk);
    delay(5000);
  }
  #ifdef DEBUG
    if(wifi_status == WL_CONNECTED){
      Serial.println("---------------\nWiFi Connected\n---------------");
    }
    else {
      Serial.println("---------------\nWiFi Could Not Connect\n---------------");
    }
  #endif
}

void get_debounced(int i) {
  buttons[i].button_state = digitalRead(button_clockwise[i]);
  if(buttons[i].button_state != buttons[i].last_button_state) {
    buttons[i].last_debounce_time = millis();
  }
  if((millis() - buttons[i].last_debounce_time) > debounce_delay && buttons[i].button_state != buttons[i].value) {
    buttons[i].value = buttons[i].button_state;
    buttons[i].last_debounce_time = 0;
    #ifdef DEBUG
      Serial.print("---------------\nDebounce Relay Button:\n");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(buttons[i].value);
      Serial.print("\n---------------\n");
    #endif
  }
}

void set_next_state(int i, int toggle) {
  if(toggle == HIGH) {
    relay_states[i].next = !relay_states[i].next;
  }
}

void get_all_relays() {
  for(int i = 0; i < num_relays; i++) {
    relay_states[i].current = digitalRead(relay_clockwise[i]);
  }
}

void set_all_relays() {
  for(int i = 0; i < num_relays; i++) {
    if(relay_states[i].current != relay_states[i].next) {
      digitalWrite(relay_clockwise[i], relay_states[i].next);
      #ifdef DEBUG
        Serial.print("---------------\nRelay State Trasition:\n");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(relay_states[i].next);
        Serial.print("\n---------------\n");
      #endif
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i = 0; i < num_relays; i++) {
    relay_states[i].current = LOW;
    relay_states[i].next = LOW;
    
    pinMode(relay_clockwise[i], OUTPUT);
    digitalWrite(relay_clockwise[i], LOW);
    
    pinMode(button_clockwise[i], INPUT);
    
    buttons[i].button_state = LOW;
    buttons[i].last_button_state = LOW;
    buttons[i].last_debounce_time = 0;
    buttons[i].value = LOW;
  }
  lockout = none;
  seq_index = 0;
  
  credentials = wpa_credentials.read()

  attempt_login();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(lockout == none) {
    get_debounced(seq_index);
    set_next_state(seq_index, buttons[seq_index].value);
  }
  
  if(lockout != network) {
    cmd = console_get_command();
    result = process_command(cmd);
    console_put_response(result);
  }
  
  if(lockout != console) {
    cmd = tcp_get_command();
    result = process_command(cmd);
    tcp_put_response(result);
  }
  
  get_all_relays();
  set_all_relays();
  seq_index++;
}
