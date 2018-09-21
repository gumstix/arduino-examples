#include <FlashAsEEPROM.h>
#include <FlashStorage.h>

#define MAX_WIFI_ATTEMPTS 5
#define DEBUG

#include <SPI.h>
#include <WiFi101.h>
#include "relay_commander.h"

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

FlashStorage(wpa_credentials, wpa_data);

lockout_value lockout;

relay_state relay_states[num_relays];
debounce_state buttons[num_relays];

wpa_data credentials;
int wifi_status = WL_IDLE_STATUS;

int seq_index;

WiFiServer server(server_port);

WiFiClient client;
////////////////////////////////////////////////////////////////////
//                     RELAY CONTROL SYSTEM                       //
////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
// SET_NEXT_STATE
// Setup value to be written to each relay

void set_next_state(int i, int toggle) {
  if(toggle == HIGH) {
    relay_states[i].next = !relay_states[i].next;
  }
}
//////////////////////////////////////////////////////////////////
// GET_ALL_RELAYS
// Get actual pin values for each relay to ensure the correct
// relays are modified when in set cycle

void get_all_relays() {
  for(int i = 0; i < num_relays; i++) {
    relay_states[i].current = digitalRead(relay_clockwise[i]);
  }
}


/////////////////////////////////////////////////////////////////
// SET_ALL_RELAYS
// Toggle relays set to change state

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

////////////////////////////////////////////////////////////////////
//                  PUSHBUTTON COMMAND SYSTEM                     //
////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// GET_DEBOUNCED
// Store stabilized value for relay pushbuttons.

void get_debounced(int i) {
  buttons[i].button_state = digitalRead(button_clockwise[i]);
  if(buttons[i].button_state != buttons[i].last_button_state) {
    buttons[i].last_debounce_time = millis();
  }
  if((millis() - buttons[i].last_debounce_time) > debounce_delay 
                  && buttons[i].button_state != buttons[i].value) {
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


////////////////////////////////////////////////////////////////////
//                    CONSOLE COMMAND SYSTEM                      //
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// IS_VALID_CMD
// Verify command

bool is_valid_cmd(char cmd) {
  for(int i = 0; i < sizeof(valid_cmds); i++) {
    if(cmd == valid_cmds[i])
      return true;
  }
  return false;
}


/////////////////////////////////////////////////
// CONSOLE_GET_COMMAND
// Collect any valid input from serial console

void console_get_command() {
  cmd[0] = 0;
  if(Serial.available()) {
    for(int i=0; Serial.available() && i < 32; i++) {
      cmd[i] = Serial.read();
      if(cmd[i] == '\n') break;
    }
    if(!is_valid_cmd(cmd[0]))
      cmd[0] = 0;
  }
  return;
}

/////////////////////////////////////////////////
// CONSOLE_PROCESS_COMMAND
// take validated command and commit it
//

void console_process_command() {
  int relay = num_relays + 1;        
  int len = 0;
  if (cmd[0] != 0) {
    char key = cmd[0];
    switch(key) {
      case 'w': // Set wifi SSID
         Serial.print("Please enter WiFi SSID:   ");
         len = Serial.readBytesUntil('\n', credentials.ssid, 33);
         credentials.ssid[len] = 0;
         #ifdef DEBUG
          Serial.print("WPA SSID = ");
          Serial.println(credentials.ssid);
        #endif
         Serial.println("\nData will not be saved to FLASH until (c)onnect!"); 
         break;
      case 'p': // Set wifi PSK
        Serial.print("Please enter WiFi PSK:   ");
        len = Serial.readBytesUntil('\n', credentials.psk, 64);
        credentials.psk[len] = 0;
        #ifdef DEBUG
          Serial.print("WPA PSK = ");
          Serial.println(credentials.psk);
        #endif
        Serial.println("\nData will not be saved to FLASH until (c)onnect!");
        break;
      case 'c': // Connect to wifi and save credentials if successful
        Serial.print("Attempting to login to WiFi Network: ");
        Serial.println(credentials.ssid);
        if(attempt_login()) {
          wpa_credentials.write(credentials);
          Serial.println("Login successful!  New credentials saved!");
        }
        else
          Serial.println("Login failed!");
        break;
      case 'l': //Lockout pushbuttons and WiFi
        if(lockout == none) {
          lockout = console;
          Serial.println("Console Lockout Enabled!");
        }
        else if(lockout == console) {
          lockout = none;
          Serial.println("Console Lockout Disabled!");
        }
        break;
      case 't': //Toggle relay
        relay = cmd_get_relay();
        if(relay < num_relays) {
          if(relay_states[relay].current == HIGH)  
            relay_states[relay].next = LOW;
          else
            relay_states[relay].next = HIGH;
        }
      case 'd':
      default:
      break;
    }
  }
}

void console_put_response() {
  return;
}


////////////////////////////////////////////////////////////////////
//                    NETWORK COMMAND SYSTEM                      //
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// ATTEMPT_LOGIN:
// Using credentials acquired from the FLASH memory, attempt to connect
// to WIFI via WPA.

bool attempt_login() {
  int attempts = 0;
  while(wifi_status != WL_CONNECTED && attempts < MAX_WIFI_ATTEMPTS) {
    wifi_status = WiFi.begin(credentials.ssid, credentials.psk);
    delay(5000);
  }
  #ifdef DEBUG
    if(wifi_status == WL_CONNECTED){
      Serial.println("---------------\nWiFi Connected\n---------------");
      server.begin();
    }
    else {
      Serial.println("---------------\nWiFi Could Not Connect\n---------------");
    }
  #endif
  return(wifi_status == WL_CONNECTED);
}


  void tcp_get_command() {
    if(!client) 
      client = server.available();
    if(client) {
      while(client.connected()) {
        if client.available() {
          
        }
      }
    }
    return;  
  }

  void tcp_process_command() {
    return;
  }

  void tcp_put_response() {
    return;
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
  
  credentials = wpa_credentials.read();

  attempt_login();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(lockout == none) {
    get_debounced(seq_index);
    set_next_state(seq_index, buttons[seq_index].value);
  }
  
  if(lockout != network) {
    console_get_command();
    console_process_command();
    console_put_response();
  }
  
  if(lockout != console) {
    tcp_get_command();
    tcp_process_command();
    tcp_put_response();
  }
  
  get_all_relays();
  set_all_relays();
  seq_index++;
}
