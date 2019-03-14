#include <FlashAsEEPROM.h>
#include <FlashStorage.h>

#define MAX_WIFI_ATTEMPTS 3

#include <SPI.h>
#include <WiFi101.h>
#include "relay_commander.h"

const int response_len = 10;
String response_str[] = {"HTTP/1.1 200 OK", "Content-Type: text/html", "Connection: close", "", "<!DOCTYPE HTML>", "<html>", "OK", "<br />", "</html>", ""};

String nop_response_str[] = {"HTTP/1.1 200 OK", "Content-Type: text/html", "Connection: close", "", "<!DOCTYPE HTML>", "<html>", "NOT AVAILABLE", "<br />", "</html>", ""};
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

boolean console_connected;

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
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////
// _PROCESS_COMMAND
// Interface-agnostic switch statement for updating relay states
// based on cmd value


void _process_command(char key, char source) {
  int len = 0;
  int relay = num_relays + 1;
  char ip_str[18];
//  char tuple[] = {0, 0, 0, 0};
//  uint8_t ip[4];
//  int i;
//  int j;
//  int k;
  IPAddress t;
  
  switch(key) {
    case 'w': // Set wifi SSID
      Serial.flush();
      Serial.print("Please enter WiFi SSID:   ");
      len = Serial.readBytesUntil('\r', credentials.ssid, 33);
      credentials.ssid[len] = 0;
        Serial.print("WPA SSID = ");
        Serial.println(credentials.ssid);
      Serial.println("\nData will not be saved to FLASH until (c)onnect!"); 
      break;
    
    
    
    case 'p': // Set wifi PSK
      Serial.flush();
      Serial.print("Please enter WiFi PSK:   ");
      len = Serial.readBytesUntil('\r', credentials.psk, 64);
      credentials.psk[len] = 0;
      Serial.println("\nData will not be saved to FLASH until (c)onnect!");
      strcpy(response,"okay");
      break;
    
    case 'i': // set IP address
      Serial.flush();
      Serial.print("Please enter IP address: ");
      len = Serial.readBytesUntil('\r', ip_str, 18);
      ip_str[len] = 0;
      if(t.fromString(ip_str))
        credentials.ip = t;
      Serial.print("IPADDR:  "); t.printTo(Serial);
//      Serial.println(ip_str);
//      Serial.println(len);
//      j = 0;
//      i = 0;
//      k = 0;
//      while(i <= len && k < 4) {
//        Serial.println(i);
//        if(ip_str[i] != '.' && j < 3 && ip_str[i] >= '0' && ip_str[i] <= '9') {
//          tuple[j] = ip_str[i];
//          Serial.println(tuple);
//          j++;
//        }
//        else if(ip_str[i] == '.' || i == len) {
//          tuple[j] = 0;
//          j = 0;
//          ip[k] = atoi(tuple);
//          Serial.print("Tuple:"); Serial.println(tuple);
//          k++;
//        }
//        else {
//          strcpy(response, "Invalid Character (Try again!)");
//          i = len;
//        }
//        i++;
//      }
//      if(k == 4) {
//        credentials.ip = IPAddress(ip[0], ip[1], ip[2], ip[3]);
//        strcpy(response, "okay");
//      }
//      else
//        strcpy(response, "Invalid IP address");
      break;
    
    case 'c': // Connect to wifi and save credentials if successful
      Serial.print("Attempting to login to WiFi Network: ");
      Serial.println(credentials.ssid);
      if(attempt_login()) {
        wpa_credentials.write(credentials);
        Serial.println("Login successful!  New credentials saved!");
        strcpy(response,"okay");
      }
      else {
        Serial.println("Login failed!");
        strcpy(response,"fail connect");
      }
      break;
    
    
    
    case 'l': //Lockout pushbuttons and WiFi
      lockout_value new_lock; 
      if(source == 'n') 
        new_lock = network;
      else if (source == 'c')
        new_lock = console;
      if(lockout == none) {
        lockout = new_lock;
        Serial.println("Lockout Enabled!");
      }
      else if(lockout == new_lock) {
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

void set_next_state(int i) {
  if(buttons[i].value == LOW && buttons[i].last_value == HIGH) {
    relay_states[i].next = !relay_states[i].current;
  }
  else
  {
    relay_states[i].next = relay_states[i].current;
  }
  buttons[i].last_value = buttons[i].value;
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
      relay_states[i].current = relay_states[i].next;
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
    buttons[i].last_button_state = buttons[i].button_state;
  }
  else if(buttons[i].last_debounce_time != 0
                  && (millis() - buttons[i].last_debounce_time) > debounce_delay 
                  && buttons[i].button_state != buttons[i].value) {
    buttons[i].value = buttons[i].button_state;
    buttons[i].last_debounce_time = 0;
  }
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
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////
// CONSOLE_GET_COMMAND
// Collect any valid input from serial console

boolean console_get_command() {
  if(Serial.available()) {
    int len = strlen(cmd);
    int cr = 0;
    if(len < 31)
      cr  = console_read(cmd);
    else {
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
 
  int relay = num_relays + 1;        
  int len = 0;
  if (strlen(cmd) > 0) {
    char key = cmd[0];
    _process_command(key, 'c');
  }
  strcpy(cmd, "");
}

/////////////////////////////////////////////////
// CONSOLE_PUT_RESPONSE
// Display console menu after processing command
//

void console_home_location() {
  Serial.write(27);   
  Serial.print("[2J");    
  Serial.write(27);
  Serial.print("[H");
}

void console_put_response() {
  console_home_location();
  Serial.println("\r\n\r\n┌─────────────────────────────────────────┐");
  Serial.println("│             Relay Commander             │");
  Serial.println("│ Options:                                │");
  Serial.println("│ --------                                │");
  Serial.println("│     l      --->  Lockout buttons and    │");
  Serial.println("│                  WiFi                   │");
  Serial.println("│     t n    --->  Toggle relay #n        │");
  Serial.println("│     d n    --->  Disable relay #n       │");
  Serial.println("│     a n    --->  Activate realy #n      │");
  Serial.println("│     w      --->  Set WiFi SSID          │");
  Serial.println("│     p      --->  Set WiFi passkey       │");
  Serial.println("│     i      --->  Set IP Address         │");
  Serial.println("│     c      --->  Connect to WiFi and    │");
  Serial.println("│                  and save SSID/PSK to   │");
  Serial.println("│                  flash memory           │");
  Serial.println("└─────────────────────────────────────────┘");
  Serial.print("\tWifi Status:    ");
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("CONNECTED");
  }
  else
    Serial.println("DISCONNECTED");
  
  Serial.print("\tIP address:     ");
  Serial.println(credentials.ip);
  Serial.print("\tSSID = ");
  Serial.println(credentials.ssid);
  Serial.print("\tLockout: ");
  if(lockout == console)
    Serial.println("Console");
  else if (lockout == network)
    Serial.println("Network");
  else
    Serial.println("None");
  
  Serial.print("\n\n$>   ");
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
  Serial.print(credentials.ip);
  WiFi.config(credentials.ip);
  int attempts = 0;
  while(wifi_status != WL_CONNECTED && attempts < MAX_WIFI_ATTEMPTS) {
    wifi_status = WiFi.begin(credentials.ssid, credentials.psk);
    attempts++;
    delay(500);
  }
    if(wifi_status == WL_CONNECTED){
     credentials.ip = WiFi.localIP();
     server.begin();
    }
   
  return(wifi_status == WL_CONNECTED);
}

boolean tcp_get_command() {
  char this_line[255];
  char c[] = {0, 0};
  boolean ret = false;
  boolean line_is_blank = true;
  strcpy(this_line, "");
  client = server.available();
  if(client) {
    while(client.connected()) {
      if(client.available()) {
        c[0] = client.read();
     
          Serial.print(c);
     
        if(c[0] == '\n') {
          if(line_is_blank == true) {
            for(int i = 0; i < response_len; i++) {
              client.println(response_str[i]);
          
                Serial.println(response_str[i]);
          
            }
            delay(1);
            client.stop();
          }
          else if(strstr(this_line, "GET") == this_line) {
            char* cmd_end = strstr(this_line, " HTTP");
            Serial.println(this_line);
            *cmd_end = 0;
            strcpy(cmd, &this_line[5]);
            unescape(cmd);
           
            Serial.print("Command: ");
             
            Serial.println(cmd);
       
            ret = true;
          }
          strcpy(this_line, "");
          line_is_blank = true;
        }
        else if(c[0] != '\r') {
          strcat(this_line, c);
          line_is_blank = false;
        }
      }
    }
    return ret;
  }
  
}

  void tcp_process_command() {
    char key = cmd[0];
  if(is_valid_cmd(key) and key != 'w' and key != 'p')
      _process_command(key, 'n');
    return;
  }

void tcp_nop_command() {
  client = server.available();
  char c;
  if(client) {
    while(client.connected()) {
      while(client.available()) {
        c = client.read();
        Serial.print(c);
      }
      for(int i = 0; i < response_len; i++) {
        client.println(nop_response_str[i]);
        Serial.println(nop_response_str[i]);
      }
      delay(1);
      client.stop();
    }
  }
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
    
    buttons[i].button_state = digitalRead(button_clockwise[i]);
    buttons[i].last_button_state = buttons[i].button_state;
    buttons[i].last_debounce_time = 0;
    buttons[i].value = buttons[i].button_state;
    buttons[i].last_value = buttons[i].value;
  }
  lockout = none;
  seq_index = 0;
  credentials.ip = INADDR_NONE;
  credentials = wpa_credentials.read();
  if(credentials.ip == INADDR_NONE)
  #ifdef IP_ADDR
    credentials.ip = IPAddress(IP_ADDR);
  #else
    credentials.ip = IPAddress(192, 168, 55, 64);
  #endif
  attempt_login();
  cli_in[0] = 0;
  cli_in[1] = 0;
  strcpy(cmd, "");
  if(Serial)
    console_connected = true;
  else
    console_connected = false;
  if(console_connected)
    console_put_response();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!console_connected and Serial) {
    console_connected = true;
    console_put_response();
        
  }
  if(console_connected and !Serial) {
    console_connected = false;
  }
  
  if(lockout == none) {
    get_debounced(seq_index);
    set_next_state(seq_index);
  }

  if(lockout != network) {
    if(console_get_command() == true) {
      console_process_command();
      strcpy(cmd, "");
      console_put_response();
    }
  }
  
  if(lockout != console) {
    if(WiFi.status() != WL_CONNECTED && lockout != none)
      lockout = none;
    else if(WiFi.status() == WL_CONNECTED) {
      if(tcp_get_command() == true) {
        tcp_process_command();
        Serial.print("wifi cmd recieved: ");
        Serial.println(cmd);
        Serial.print("Response: ");
        Serial.println(response);
        strcpy(cmd, "");
      }
    }    
  }

  if(lockout == console) {
    tcp_nop_command();
  }
  
  get_all_relays();
  set_all_relays();
  seq_index++;
  if(seq_index >= num_relays) seq_index = 0;
}
