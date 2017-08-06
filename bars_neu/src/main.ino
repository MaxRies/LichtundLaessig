#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "Bar.h"
#define LED_PIN D7
#define NUM_LEDS 28
#define DATA_PIN D7
#define LED_STATUS_PIN 1
#define MAX_PACKET_SIZE 32

#define debug

CRGB leds[NUM_LEDS];
Pattern pattern(leds, NUM_LEDS);

boolean animal_selected = false;
boolean animation_running = false;

int status = WL_IDLE_STATUS;

IPAddress remoteIP;

timeMeasurer mess;

subscribeMessage subMesg;
synchronisingMessage syncMesg;
settingMessage setMesg;
pushingMessage pushMesg;
statusingMessage statMesg;

long millisSinceSync;
long lastSync;
long millisSinceBeat;
long lastBeat;
long millisSinceRequest;
long lastRequest;
long now;
long lastSave;

double x;

bool waitingForAck;
bool registered;

//#define debug

// benötigte Objekte
WiFiClient espClient;
PubSubClient client(espClient);

// Devicespezifischer Kram
Bar dev(client);
const char* type = "StandardLight";
// int typeLen = 17;

// Kommunikationskram
IPAddress mqtt_server(192,168,0,2);
const char* ssid_start = "llbrain";
const char* pw = "lichtundlaessig!";


// Globale Variablen
char macChar[13];
char wifiSSID[20];

// Flags
bool connectedToRightWifi = false;
bool completedHandshake = false;

// Triggervariablen
unsigned long last_trigger = 0;

int current_base_pattern = 1;
int current_front_pattern = 2;
int current_strobe = 2;
int color_picker = 0;
int base_color = 128;
int front_color = 220;
int strobe_color = 128;
int main_dimmer = 255;
bool strobe_on = false;
bool light_on = false;

int farben[90][2] = {{1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {2, 1},
  {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {2, 10}, {3, 1}, {3, 2},
  {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10}, {4, 1}, {4, 2}, {4, 3},
  {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9}, {4, 10}, {5, 1}, {5, 2}, {5, 3}, {5, 4},
  {5, 6}, {5, 7}, {5, 8}, {5, 9}, {5, 10}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5},
  {6, 7}, {6, 8}, {6, 9}, {6, 10}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6},
  {7, 8}, {7, 9}, {7, 10}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7},
  {8, 9}, {8, 10}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8},
  {9, 10}, {10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 5}, {10, 6}, {10, 7}, {10, 8}, {10, 9}
};

int patterns[42][2] = {{1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{2,1},{2,2},{2,3},
{2,4},{2,5},{2,6},{2,7},{3,1},{3,2},{3,3},{3,4},{3,5},{3,6},{3,7},{4,1},{4,2},
{4,3},{4,4},{4,5},{4,6},{4,7},{5,1},{5,2},{5,3},{5,4},{5,5},{5,6},{5,7},{6,1},
{6,2},{6,3},{6,4},{6,5},{6,6},{6,7}};

void buildFakeMessage() {
  setMesg.strobePattern = current_strobe; // from 1 to 265
  setMesg.strobeColor = strobe_color;
  setMesg.strobeBrightness = 255;
  setMesg.basePattern = current_base_pattern;
  setMesg.frontPattern = current_front_pattern;
  setMesg.mainDim = main_dimmer;
  setMesg.frontColor = front_color;
  setMesg.baseColor = base_color;
  setMesg.baseSpeed = 200;
  setMesg.frontSpeed = 70;
  setMesg.strobeSpeed = 100;
  setMesg.strobeBrightness = 255;
  setMesg.dutyCycle = 20;
  setMesg.frontBrightness = 255;
  setMesg.baseBrightness = 255;
}

void setFakeMessage() {
  pattern.setNbasePattern((double) setMesg.basePattern);
	pattern.setNbaseColor((double) setMesg.baseColor);
	pattern.setNbaseSpeed((double) setMesg.baseSpeed);
	pattern.setNbaseDim((double) setMesg.baseBrightness);
	pattern.setNfrontPattern((double) setMesg.frontPattern);
	pattern.setNfrontColor((double) setMesg.frontColor);
	pattern.setNfrontSpeed((double) setMesg.frontSpeed);
	pattern.setNfrontDim((double) setMesg.frontBrightness);
	pattern.setNstrobePattern((double) setMesg.strobePattern);
	pattern.setNstrobeColor((double) setMesg.strobeColor);
	pattern.setNstrobeSpeed((double) setMesg.strobeSpeed);
	pattern.setNstrobeDim((double) setMesg.strobeBrightness);
	pattern.setDimVal((double) setMesg.mainDim);
	pattern.setDutyCycle(((double) setMesg.dutyCycle ));
	pattern.setSettings();
	pattern.setStrobeStart(millis());
}

void stuff_to_trigger(){
  if (millis() > last_trigger + 500) {
  last_trigger = millis();
  pattern.setBeatPeriodMillis(500);
  pattern.setBeatDistinctiveness(100);
  pattern.setMillisSinceBeat(0);
  millisSinceBeat = 0;
  lastBeat = millis();

  if(light_on){
    pattern.baseChoser();
    pattern.frontChoser();
  }
  if(strobe_on){
    Serial.print("Strobe on.");
    pattern.fillBlack();
    pattern.strobeChoser();
  }
  FastLED.setCorrection(TypicalSMD5050);
  if(!light_on && !strobe_on){
    pattern.fillBlack();
  }
  FastLED.show((uint8_t)pattern.getDimVal());
}
}

void debugMesg(const char* msg) {
#ifdef debug
	Serial.println(msg);
#endif
}

bool subscribe_to_topics(){
      bool error = false;
      char c_number[4];
      int num = dev.getNumber();
      snprintf(c_number, 4, "%d", num);
      char topicBuffer[200] = "devices/StandardLight/";
      strcat(topicBuffer, c_number);
      strcat(topicBuffer, "/#");
      #ifdef debug
      Serial.print("Subscribed to: ");
      Serial.println(topicBuffer);
      #endif
      return dev._client.subscribe(topicBuffer);
}

void subscribe_for_login(){
  char topicBuffer[200] = "hello/";
  strcat(topicBuffer, macChar);
  strcat(topicBuffer, "/id");
  client.subscribe(topicBuffer);
}

void callback_for_login(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i=0;i<length;i++) {
      msg += (char)payload[i];
    }
  #ifdef debug
    Serial.println(msg);
  #endif
  int msgInt = msg.toInt();

  char topicBuffer[200] = "hello/";
  strcat(topicBuffer, macChar);
  strcat(topicBuffer, "/id");

  if (strcmp(topic,topicBuffer)==0){
    if (msgInt == -1) {
      // falsches WiFi, also raus hier
      connectedToRightWifi = false;
      completedHandshake = true;
      #ifdef debug
      Serial.println("wrong WiFi.");
      #endif
    }
    else {
      completedHandshake = true;
      connectedToRightWifi = true;
      dev.setNumber(msgInt);
      #ifdef debug
      Serial.print("Right WiFi. Device Number: ");
      Serial.println(dev.getNumber());
      #endif
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef debug
    Serial.print("Attempting MQTT connection...");
    #endif
    // Create a random client ID
    String clientId = dev.getType();
    clientId += "-";
    clientId += WiFi.macAddress();
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      #ifdef debug
      Serial.println("connected");
      #endif
      // HIER NEU SUBSCRIBEN!
      if (!connectedToRightWifi) {
        subscribe_for_login();
      }
      else {
        subscribe_to_topics();
      }
    } else {
      #ifdef debug
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      #endif
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}


bool try_to_login(char *SSID){
  /*
  Ablauf:
  Wifi SSID connecten
  Mac und Device Type an hello topic schicken
  warten, was auf dem topic /hello/[mac] zurückkommt
  Wenn 0: Handshake complete und connectedToRightWifi = false
  wenn n: n als device ID speichern, callback des hauptprogramms
          für den MQTT Broker hinterlegen.
  */
  WiFi.begin(SSID, pw);

  #ifdef debug
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
  #endif
  fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  FastLED.show();
  while (WiFi.status() != WL_CONNECTED){
    #ifdef debug
      Serial.print(".");
    #endif

      delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback_for_login);
  reconnect();

  // Hello Topic zusammenbauen
  char loginTopic[200] = "hello/";
  strcat(loginTopic, macChar);
  strcat(loginTopic, "/type");
  client.publish(loginTopic, type);

  #ifdef debug
    Serial.println(loginTopic);
    Serial.print("waiting for handshake to complete.");
  #endif
  int killcount = 0;
  while (completedHandshake == false){
    delay(500);
    client.loop();
    #ifdef debug
      Serial.print(".");
    #endif
    killcount++;
    if (killcount > 5) {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.show();
      connectedToRightWifi = false;
      completedHandshake = false;
      #ifdef debug
      Serial.println("Handshake failed. Next WiFi...");
      #endif
      break;
    }
  }
  completedHandshake = false;
  return connectedToRightWifi;

}

void login(){
  int nWifis = WiFi.scanNetworks();
  #ifdef debug
    Serial.println(nWifis);
  #endif
    for(int wifi = 0; wifi < nWifis; wifi++)
    {
      String potSSID = WiFi.SSID(wifi);
      potSSID.toCharArray(wifiSSID, 20);
      #ifdef debug
      Serial.println(potSSID);
      #endif
      if (strncmp(wifiSSID, ssid_start,7) == 0) {
        // Start Login Routine
        #ifdef debug
        Serial.println("MATCH!");
        #endif
        bool rightWifi = try_to_login(wifiSSID);
        if (rightWifi == true) {
          break;
        }
        else
        {
          WiFi.disconnect();
          #ifdef debug
          Serial.println("disconnecting.");
          #endif
        }

      }
      else {
        #ifdef debug
        Serial.println("NO MATCH!");
        #endif
      }
    }
}

void getMacAndClientID() {
  byte myMac[6];
  WiFi.macAddress(myMac);
  snprintf(macChar, 13, "%02X%02X%02X%02X%02X%02X\0", myMac[0],myMac[1],myMac[2],myMac[3],myMac[4],myMac[5]);
}


////////////////////////////////////////////////////////////////////////////////
// Hier noch die richtigen Flags setzen für die Main Loop.
////////////////////////////////////////////////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  char setMesgPayload[length];
  for (int i=0;i<length;i++) {
      msg += (char)payload[i];
    }

  char * subTopic = strrchr(topic, '/');
  #ifdef debug
  Serial.print(subTopic);
  Serial.print("\t");
  Serial.println(msg);
  #endif

  if (strcmp(subTopic, "/enable_light")==0){
    if (msg.toInt() == 0) {
      light_on = false;
    }
    else {
      light_on = true;
      Serial.println("Light on.");
    }
}
else if (strcmp(subTopic, "/enable_strobe") == 0) {
  if (msg.toInt() == 0) {
    strobe_on = false;
  }
  else {
    strobe_on = true;
  }
  buildFakeMessage();
  setFakeMessage();
}
else if (strcmp(subTopic, "/strobe") == 0) {
  // Strobomodi von 1 bis 4, aber 1 ist quasi aus, also + 1 damit einfacher
  current_strobe = msg.toInt();
  buildFakeMessage();
  setFakeMessage();
}
else if (strcmp(subTopic, "/pattern") == 0) {
  int n = msg.toInt();
  current_base_pattern = patterns[n][0];
  current_front_pattern = patterns[n][1];
  buildFakeMessage();
  setFakeMessage();
}
else if (strcmp(subTopic, "/color") == 0) {
  int n = msg.toInt();
  front_color = farben[n][0];
  base_color = farben[n][1];
  strobe_color = farben[n][0];
  buildFakeMessage();
  setFakeMessage();
}
else if (strcmp(subTopic, "/dim") == 0) {
  main_dimmer = msg.toInt();
  buildFakeMessage();
  setFakeMessage();
}

}

void setup() {
  // Outputs initialisieren
  #ifdef debug
  Serial.begin(115200);
  Serial.println("Hello!");
  Serial.print("Device type: ");
  Serial.println(dev.getType());
  #endif
  pinMode(DATA_PIN, OUTPUT);
	digitalWrite(DATA_PIN, LOW);
	debugMesg("Setting up Leds");
	FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
  // Variablen setzen
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(500);
  #ifdef debug
  Serial.println("Setting Variables...");
  #endif
  getMacAndClientID();


  // W-Lan aufsetzen
  #ifdef debug
  Serial.println("Looking for Brain...");
  #endif

  login();

  while (!connectedToRightWifi) {
    #ifdef debug
    Serial.println("No WiFi found. Restarting login routine...");
    #endif
    FastLED.show(10);
    delay(500);
    FastLED.show(255);
    delay(500);
    FastLED.show(10);
    delay(500);
    FastLED.show(255);
    login();
  }
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();


	//Set vars

	millisSinceSync = 0;
	lastSync = 0;
	millisSinceBeat = 0;
	lastBeat = 0;
	millisSinceRequest = 0;
	lastRequest = 0;
	now = 0;

	waitingForAck = false;
	registered = false;

	FastLED.setCorrection(TypicalSMD5050);
	FastLED.show();
	pattern.savedVals.read();
	lastSave = 0;
	debugMesg("Setup finished");

/*
  String hostname = String("bar-");
  hostname += String(dev.getNumber());
  char buffer[30];
  hostname.toCharArray(buffer, 30);
  ArduinoOTA.setHostname(buffer);*/

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
  });
  ArduinoOTA.onEnd([]() {

  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

  });
  ArduinoOTA.onError([](ota_error_t error) {

  });
  ArduinoOTA.begin();

  // Neue topics SUBSCRIBEN
  #ifdef debug
  Serial.println("Subscribing to Strobo topics...");
  #endif
  subscribe_to_topics();
  client.setCallback(callback);
}

void setTimes() {
	now = millis();
	millisSinceSync = now - lastSync;
	millisSinceBeat = now - lastBeat;
	millisSinceRequest = now - lastRequest;
	pattern.setMillisSinceBeat((double) millisSinceBeat);
	pattern.setStrobeTime(millis()- pattern.getStrobeStart());
}

void loop() {
  // Haben wir unser W-Lan noch?
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef debug
    Serial.println("W-Lan verloren, reset!");
    #endif
    ESP.restart();
  }
  // Connection zu Broker noch da?
  if (!client.connected()) {
    reconnect();
  }

  // MQTT Client loopen
  client.loop();

  // BARs STUFF
  setTimes();
  stuff_to_trigger();
	if(millis() - lastSave > 30000){
		pattern.savedVals.write();
		lastSave = millis();
	}
  if(light_on){
    pattern.baseChoser();
  	pattern.frontChoser();
  }
  if(strobe_on){
    pattern.fillBlack();
    pattern.strobeChoser();
  }
	FastLED.setCorrection(TypicalSMD5050);
  if(!light_on && !strobe_on){
    pattern.fillBlack();
  }
  FastLED.show((uint8_t)pattern.getDimVal());
  ArduinoOTA.handle();

}
