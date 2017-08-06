#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "Seilwinde.h"

//#define debug

// benötigte Objekte
WiFiClient espClient;
PubSubClient client(espClient);

// Devicespezifischer Kram
Seilwinde dev(client);
const char* type = "Seilwinde";

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
    dev.loop();
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
        dev.subscribe_to_topics();
      }
    } else {
      #ifdef debug
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      #endif
      // Wait 1 second before retrying
      unsigned long enter = millis();
      while (millis() < enter + 100) {
        dev.loop();
      }
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

  while (WiFi.status() != WL_CONNECTED){
    #ifdef debug
      Serial.print(".");
    #endif
      unsigned long enter = millis();
      while(millis() < enter + 500) {
        dev.loop();
      }
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
  unsigned long enter = millis();
  while (completedHandshake == false){
    client.loop();
    dev.loop();
    #ifdef debug
      Serial.print(".");
    #endif
    yield();
    if (millis() > enter + 10000) {
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
  for (int i=0;i<length;i++) {    // Umbau der MSG in Arduino-String Objekt
      msg += (char)payload[i];
    }

  char * subTopic = strrchr(topic, '/');  // pointer auf letzten Teil nach dem "/"
  #ifdef debug
  Serial.print(subTopic);
  Serial.print("\t");
  Serial.println(msg);
  #endif

  if (strcmp(subTopic, "/drop")==0){    // strcmp liefert 0 wenn strings identisch
      dev.drop(msg.toInt());
  }
  else if (strcmp(subTopic, "/lift") == 0){
    dev.pull(msg.toInt());
  }
  else if (strcmp(subTopic, "/stop") == 0) {
    dev.stop();
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

  dev.setup();

  // Variablen setzen
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
    Serial.println("No WiFi found. Restarting login...");
    #endif
    login();
  }

  // Neue topics SUBSCRIBEN
  #ifdef debug
  Serial.println("Subscribing to Device topics...");
  #endif
  Serial.println(dev.subscribe_to_topics());
  client.setCallback(callback);
  Serial.println("Entering main loop...");
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

  // Knopfdev loopen!
  dev.loop();

}
