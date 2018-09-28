#include <FlashAsEEPROM.h>
#include <FlashStorage.h>

#define MAX_WIFI_ATTEMPTS 5
#define DEBUG

#include <SPI.h>
#include <WiFi101.h>
#include "relay_commander.h"
const char response_str[] = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n<!DOCTYPE HTML>\n<html>OK<br /></html>";
int relay_clockwise[] =   {
//                            PB3,
//                            PA2,
//                            PA4,
//                            PA6,
//                            PA8,
//                            PA10,
//                            PA16,
//                            PA19,
//                            PA21,
//                            PA23
                              0,
                              1,
                              2
                          };
int button_clockwise[] =  {
//                            PB10,
//                            PA3,
//                            PA5,
//                            PA7,
//                            PA9,
//                            PA11,
//                            PA17,
//                            PA20,
//                            PA22,
//                            PB2
                              0,
                              1,
                              2
                          };

FlashStorage(wpa_credentials, wpa_data);

lockout_value lockout;

relay_state relay_states[num_relays];
debounce_state buttons[num_relays];

wpa_data credentials;
int wifi_status = WL_IDLE_STATUS;

int seq_index;

char cli_in[2];

WiFiServer server(server_port);

WiFiClient client;

////////////////////////////////////////////////////////
// IS_VALID_CMD
// Verify command

bool is_valid_cmd(char c) {
  for(int i = 0; i < n_valid_cmds; i++) {
    if(c == valid_cmds[i]){
      #ifdef DEBUG
        Serial.println("VALID_COMMAND!");
      #endif
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////
// _PROCESS_COMMAND
// Interface-agnostic switch statement for updating relay states
// based on cmd value


void _process_command(char key) {
  int len = 0;
  int relay = num_relays + 1;
  switch(key) {
    
    case 'w': // Set wifi SSID
      Serial.print("Please enter WiFi SSID:   ");
      len = Serial.readBytesUntil('\r', credentials.ssid, 33);
      credentials.ssid[len] = 0;
      #ifdef DEBUG
        Serial.print("WPA SSID = ");
        Serial.println(credentials.ssid);
      #endif
      Serial.println("\nData will not be saved to FLASH until (c)onnect!"); 
      break;
    
    
    
    case 'p': // Set wifi PSK
      Serial.flush();
      Serial.print("Please enter WiFi PSK:   ");
      len = Serial.readBytesUntil('\r', credentials.psk, 64);
      credentials.psk[len] = 0;
      #ifdef DEBUG
        Serial.print("WPA PSK = ");
        Serial.println(credentials.psk);
      #endif
      Serial.println("\nData will not be saved to FLASH until (c)onnect!");
      strcpy(response,"okay");
      break;
    
    
    
    case 'c': // Connect to wifi and save credentials if successful
      Serial.print("Attempting to login to WiFi Network: ");
      Serial.println(credentials.ssid);
//      if(attempt_login()) {
//        wpa_credentials.write(credentials);
//        Serial.println("Login successful!  New credentials saved!");
//        strcpy(response,"okay");
//      }
//      else {
//        Serial.println("Login failed!");
//        strcpy(response,"fail connect");
//      }
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
      strcpy(response,"okay");
      break;
    
    
    
    case 't': //Toggle relay
      relay = cmd_get_relay();
      if(relay < num_relays) {
        if(relay_states[relay].current == HIGH)  
          relay_states[relay].next = LOW;
        else
          relay_states[relay].next = HIGH;
        strcpy(response,"okay");
      }
      else
        strcpy(response,"fail range");
      break;
      
    
    
    case 'a': //Activate relay
      relay = cmd_get_relay();
      if(relay < num_relays) {
        relay_states[relay].next = HIGH;
        strcpy(response,"okay");
      }
      else
        strcpy(response,"fail range");
      break;


    
    case 'd': //Deactivate relay
      relay = cmd_get_relay();
      if(relay < num_relays) {
        relay_states[relay].next = LOW;
        strcpy(response,"okay");
      }
      else
        strcpy(response,"fail range");
      break;
      default:
        strcpy(response,"fail cmd");
      break;
  }
}



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
        relay_states[i].next = relay_states[i].current;
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
//  buttons[i].button_state = digitalRead(button_clockwise[i]);
//  if(buttons[i].button_state != buttons[i].last_button_state) {
//    buttons[i].last_debounce_time = millis();
//  }
//  if((millis() - buttons[i].last_debounce_time) > debounce_delay 
//                  && buttons[i].button_state != buttons[i].value) {
//    buttons[i].value = buttons[i].button_state;
//    buttons[i].last_debounce_time = 0;
//    #ifdef DEBUG
//      Serial.print("---------------\nDebounce Relay Button:\n");
//      Serial.print(i);
//      Serial.print(": ");
//      Serial.print(buttons[i].value);
//      Serial.print("\n---------------\n");
//    #endif
//  }
}


////////////////////////////////////////////////////////////////////
//                    CONSOLE COMMAND SYSTEM                      //
////////////////////////////////////////////////////////////////////

////////////////////////////////////
// CONSOLE_READ
// Get command from cmd line
int console_read(char* str) {
  cli_in[0] = Serial.read();
  Serial.print(cli_in[0]);
  strcat(cmd, cli_in);
  if(cli_in[0] == '\r') {
    #ifdef DEBUG
      Serial.println("cr EOL!");
    #endif
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////
// CONSOLE_GET_COMMAND
// Collect any valid input from serial console

boolean console_get_command() {
  if(Serial.available()) {
    #ifdef DEBUG
      if(strlen(cmd) == 0)
        Serial.println("Console Get Command:");
    #endif
    int len = strlen(cmd);
    #ifdef DEBUG
      Serial.println(len);
    #endif
    int cr = 0;
    if(len < 31)
      cr  = console_read(cmd);
    else {
      #ifdef DEBUG
        Serial.println(cmd);
      #endif
      cr = 1;
    }
    if(cr == 1) {
      if(!is_valid_cmd(cmd[0]))
        strcpy(cmd, "");
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////
// CONSOLE_PROCESS_COMMAND
// take validated command and commit it
//

void console_process_command() {
  #ifdef DEBUG
    Serial.print("Process Command: ");
    Serial.println(cmd);
  #endif
  int relay = num_relays + 1;        
  int len = 0;
  if (strlen(cmd) > 0) {
    char key = cmd[0];
    _process_command(key);
  }
  strcpy(cmd, "");
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
    #ifdef DEBUG
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(credentials.ssid);
    #endif
    wifi_status = WiFi.begin(credentials.ssid, credentials.psk);
    attempts++;
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
  char this_line[255];
  char c[] = {0, 0};
  strcpy(this_line, "");
  if(!client) 
    client = server.available();
  if(client) {
    while(client.connected()) {
      if(client.available()) {
        c[0] = client.read();
        if(c != "\r")
          strcat(this_line, c);
        if(c == "\n") {
          if(strcmp(this_line, c)) {
            tcp_put_response(client);
            break;
          }
          if(strstr(this_line, "GET") == this_line) {
            char* cmd_end = strstr(this_line, " HTTP");
            *cmd_end = 0;
            strcpy(cmd, &this_line[5]);
            #ifdef DEBUG
              Serial.print("Command: ");
              Serial.println(cmd);
            #endif
          }
          strcpy(this_line, "");
        }
      }
    }
  }
  return;  
}

  void tcp_process_command() {
    #ifdef DEBUG
      Serial.println("TCP Process Command!");
    #endif
    char key = cmd[0];
    if(!is_valid_cmd(key) or key == 'w' or key == 'p')
    {
      strcpy(cmd, "");
    }
    else
      _process_command(key);
  }

  void tcp_put_response(WiFiClient client) {
    client.println(response_str);
    return;
  }






  

////////////////////////////////////////////////////////////////////
//                            MAIN                                //
////////////////////////////////////////////////////////////////////










  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(9999);
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
  
  //credentials = wpa_credentials.read();

  //attempt_login();
  cli_in[0] = 0;
  cli_in[1] = 0;
  strcpy(cmd, "");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(lockout == none) {
    get_debounced(seq_index);
    set_next_state(seq_index, buttons[seq_index].value);
  }
  
  if(lockout != network) {
    if(console_get_command() == true) {
      #ifdef DEBUG
        Serial.print("cmd recieved: ");
        Serial.println(cmd);
      #endif
      console_process_command();
      strcpy(cmd, "");
    }
    console_put_response();
  }
  
  if(lockout != console) {
    if(WiFi.status() != WL_CONNECTED && lockout != none)
      lockout = none;
    else if(WiFi.status() == WL_CONNECTED) {
      tcp_get_command();
      tcp_process_command();
    }    
  }
  
  get_all_relays();
  set_all_relays();
  seq_index++;
  if(seq_index >= num_relays) seq_index = 0;
}
