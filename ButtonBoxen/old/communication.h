#ifndef communication_h
#define communication_h

#include <Arduino.h>

const char* ssid_start = "llbrain";
const char* pw = "lichtundlaessig!";
const char* mqtt_server = "192.168.1.1";
bool connectedToRightWifi = false;
bool completedHandshake = false;
byte myMac[6];
char macChar[13];


void setup_wifi();
void find_wifis();
void setup_mqtt();
void callback(char* topic, byte* payload, unsigned int length);
bool subscribe_to_topics(char* type_of_device);


#endif
