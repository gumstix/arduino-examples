#ifndef __RELAY_COMMANDER_H__
#define __RELAY_COMMANDER_H__

#define MAX_WIFI_ATTEMPTS 5
#define DEBUG

const int server_port = 80;

const int num_relays = 10;

const unsigned long debounce_delay = 50;

typedef struct relay_state_t {
  int current;
  int next; 
} relay_state;

typedef struct wpa_data_t {
  char ssid[33];
  char psk[64];
} wpa_data;

typedef struct debounce_state_t {
  int button_state;
  int last_button_state;
  unsigned long last_debounce_time;
  int value;
  int last_value;
} debounce_state;

typedef enum lockout_value_t {none, console, network} lockout_value;

typedef enum serial_cmd_t { no_cmd, 
                            set_wifi_ssid = 'w', 
                            set_wifi_psk = 'p', 
                            connect_wifi = 'c', 
                            disconnect_wifi = 'd', 
                            enable_relay = 'e', 
                            disable_relay = 'd',
                            lockout_serial = 'l'
                          } serial_cmd;


const int n_valid_cmds = 7;
char valid_cmds[] = { 
                      'w', // Set WiFi SSID
                      'p', // Set WiFi PSK
                      'c', // Attempt to connect to WiFi
                      'l', // Lockout
                      't', // Toggle relay
                      'a', // Activate relay
                      'd'  // Deactivate relay
                    };


char cmd[32];
char response[64];

int cmd_get_relay() {
  int relay = num_relays + 1;
  int i = 1;
  while(relay > num_relays && i <=32) {
    if(cmd[i] >= '0' && cmd[i] < ('0' + num_relays)) {
            relay = atoi(&(cmd[i]));
            #ifdef DEBUG
              Serial.print("Relay #: ");
              Serial.println(relay);
            #endif
          }
    else if(cmd[i] == 0 || cmd[i] =='\n')
      break;
    i++;
  }
  return relay;
}

#endif
