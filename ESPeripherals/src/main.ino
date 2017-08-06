#include <ESP8266WiFi.h>
#include "knopfbox.h"

#define debug

// Devicespezifischer Kram
// Device-Objekt muss dev heißen damit der Rest klappt!
Knopfbox dev;
const char* type = "knopfbox";
int typeLen = 8;

// Kommunikationskram
IPAddress mqtt_server(192,168,178,58);
const char* ssid_start = "FRITZ!Box 7490";
const char* pw = "ak47b00anull00";


// Globale Variablen
char macChar[13];
char wifiSSID[20];
char helloTopic[19];

// Flags
bool connectedToRightWifi = false;
bool completedHandshake = false;

// benötigte Objekte
WiFiClient espClient;
PubSubClient client(espClient);

void callback_for_login(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i=0;i<length;i++) {
      msg += (char)payload[i];
    }
  #ifdef debug
    Serial.println(msg);
  #endif
  int msgInt = msg.toInt();
  if (strcmp(topic,helloTopic)==0){
    if (msgInt == 0) {
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
      // TODO: HIER NEU SUBSCRIBEN!
    } else {
      #ifdef debug
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      #endif
      // Wait 5 seconds before retrying
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
  memcpy(helloTopic, "hello/", 6);
  memcpy(&helloTopic[6], macChar, 13);

  client.publish(helloTopic, "1");
  client.subscribe(helloTopic);

  #ifdef debug
    Serial.print("waiting for handshake to complete.");
  #endif

  while (completedHandshake == false){
    delay(500);
    client.loop();
    #ifdef debug
      Serial.print(".");
    #endif
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
  snprintf(macChar, 13, "%02X%02X%02X%02X%02X%02X\n", myMac[0],myMac[1],myMac[2],myMac[3],myMac[4],myMac[5]);
  /*memcpy(clientID, type, typeLen);
  memcpy(&clientID[typeLen], macChar, 13);
  #ifdef debug
  Serial.println(clientID);
  #endif*/
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Hier noch die richtigen Flags setzen für die Main Loop.
// TODO TOPICS ABKLÄREN!!!!
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i=0;i<length;i++) {
      msg += (char)payload[i];
    }
  #ifdef debug
    Serial.println(msg);
  #endif

  if (strcmp(topic, "")==0){


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

  // !!!! DEVICE SETUP
  dev.setup();

  // Variablen setzen
  #ifdef debug
  Serial.println("Setting Variables...");
  #endif
  getMacAndClientID();


  // W-Lan aufsetzen
  #ifdef debug
  Serial.println("Connecting to Brain...");
  #endif

  login();

  // neuen callback aufsetzen

}

void loop() {
  // Haben wir unser W-Lan noch?
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef debug
    Serial.println("W-Lan verloren, reset!");
    #endif
    ESP.reset();
  }
  // Connection zu Broker noch da?
  if (!client.connected()) {
    reconnect();
  }

  // MQTT Client loopen
  client.loop();

  // Device loopen!
  dev.loop();

}
