#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Pattern_Strobo.h"

#define debug

// benötigte Objekte
WiFiClient espClient;
PubSubClient client(espClient);

// Devicespezifischer Kram
Pattern_Strobo dev(client);
const char* type = "Strobo";

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
      connectedToRightWifi = false;
      completedHandshake = true;
      #ifdef debug
      Serial.println("Handshake failed. Next WiFi...");
      #endif
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

  if (strcmp(subTopic, "/enable_strobe")==0){    // strcmp liefert 0 wenn strings identisch
  	if(msg.toInt() == 1)
    	{
    		dev.stroboActive = true;
    	}
  	else
    	{
    		dev.stroboActive = false;
    	}
  }
  else if (strcmp(subTopic, "/enable_light") == 0){
  	if(msg.toInt() == 1)
  	{
  		dev.lightActive = true;
  	}
  	else
  	{
  		dev.lightActive = false;
  	}
  }
  else if (strcmp(subTopic, "/pattern") == 0){
	dev.patternNumber = msg.toInt();
  }
  else if (strcmp(subTopic, "/strobe") == 0){
	dev.strobe_color = msg.toInt();
  }
  else if (strcmp(subTopic, "/color") == 0){
	dev.set_pattern_color(msg.toInt());
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

  dev.login_light(1);

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

  if (!connectedToRightWifi) {
    #ifdef debug
    Serial.println("No WiFi found. Resetting ESP");
    #endif
    login();
  }

  dev.login_light(2);

  // Neue topics SUBSCRIBEN
  #ifdef debug
  Serial.println("Subscribing to Device topics...");
  #endif
  Serial.println(dev.subscribe_to_topics());
  client.setCallback(callback);
  Serial.println("Entering main loop...");
  dev.login_light(8);
  dev.login_light(100);
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

  // Knopfdev loopen!
  dev.loop();

}
