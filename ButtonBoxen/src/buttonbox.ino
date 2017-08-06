#include <ESP8266WiFi.h>
#include "knopfbox.h"

//#define debug

// benötigte Objekte
WiFiClient espClient;
PubSubClient client(espClient);

// Devicespezifischer Kram
Knopfbox box(client);
const char* type = "StandardButtonBox";
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
      box.setNumber(msgInt);
      #ifdef debug
      Serial.print("Right WiFi. Device Number: ");
      Serial.println(box.getNumber());
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
    String clientId = box.getType();
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
        box.subscribe_to_topics();
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
    box.flash_on();
    box.loop();
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
    box.loop();
    client.loop();
    #ifdef debug
      Serial.print(".");
    #endif
    killcount++;
    if (killcount > 5) {
      connectedToRightWifi = false;
      completedHandshake = false;
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
  for (int i=0;i<length;i++) {
      msg += (char)payload[i];
    }

  char * subTopic = strrchr(topic, '/');
  #ifdef debug
  Serial.print(subTopic);
  Serial.print("\t");
  Serial.println(msg);
  #endif

  if (strcmp(subTopic, "/vibe")==0){
      if (msg.toInt() == 1) {
        box.vibe_on();
        #ifdef debug
        Serial.println("Vibe on!");
        #endif
      }
      else {
        box.vibe_off();
        #ifdef debug
        Serial.println("Vibe off!");
        #endif
      }
  }
  else if (strcmp(subTopic, "/vibe_n_times")==0){
      box.vibe_times(msg.toInt());
  }
  else if (strcmp(subTopic, "/beep") == 0){
    if (msg.toInt() == 1) {
      box.beep_on();
    }
    else {
      box.beep_off();
    }
  }
  else if (strcmp(subTopic, "/beep_n_times") == 0){
    box.beep_times(msg.toInt());
  }
  else if (strcmp(subTopic, "/light") == 0){
    if (msg.toInt() == 1) {
      box.lights_on();
    }
    else {
      box.lights_off();
    }
  }
  else if (strcmp(subTopic, "/flash") == 0){
    if (msg.toInt() == 1) {
      box.flash_on();
    }
    else {
      box.flash_off();
    }
  }
  else if (strcmp(subTopic, "/flash_n_times") == 0){
    box.flash(msg.toInt());
  }
}

void setup() {
  // Outputs initialisieren
  #ifdef debug
  Serial.begin(115200);
  Serial.println("Hello!");
  Serial.print("Device type: ");
  Serial.println(box.getType());
  #endif

  box.pinSetup();

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

  // Neue topics SUBSCRIBEN
  #ifdef debug
  Serial.println("Subscribing to Knopfbox topics...");
  #endif
  Serial.println(box.subscribe_to_topics());
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

  // Knopfbox loopen!
  box.loop();

}
