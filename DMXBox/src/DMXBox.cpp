/*
 * Autor: Simon Fritz
 * und Max.
 */
#include "DMXBox.h"
#include <PubSubClient.h>
#include "ESPDMX.h"

DMXESPSerial dmx;

DMXBox::DMXBox(PubSubClient client)
  : Device ("DMXBox"){
    this->_client = client;
}

///////////////// DMXBox /////////////////

// First channel is 1 not 0
void DMXBox::update(String msg){ // updates DMX according to msg "channel1, value1, channel2, value2..."

  int Index = msg.indexOf(',');             //finds location of first ','

  while(Index != -1){   // 0 or null means no character found
    int channel = msg.substring(0, Index).toInt();  // reads channel
    msg = msg.substring(Index + 1);     // overwrite string with the rest-string
    Index = msg.indexOf(',');           // finds location of next ','
    int value = msg.substring(0, Index).toInt();    // reads value
    dmx.write(channel, value);    // write value to DMX-Data
    msg = msg.substring(Index + 1);     // overwrite string with the rest-string
    Index = msg.indexOf(',');           // finds location of next ','
  }
  dmx.update(); // update all new values
  delay(200);   // wait 200ms
}

void DMXBox::kill(){
  for(int i = 1; i <= dmx.lastAdress; i++)
  {
    dmx.write(i, 0);  // stets channel i to 0
  }
  dmx.update(); // update all new values
  delay(200);   // wait 200ms
}

void DMXBox::BlickOneSecond(){
  digitalWrite(D1, HIGH);           // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(D1, LOW);    // turn the LED off by making the voltage LOW
}

///////////////// loop /////////////////

void DMXBox::loop(){
  unsigned long now = millis();
  if (now > last_light + 500) {
    last_light = now;
    if (light_on){
      digitalWrite(D1, LOW);
      light_on = false;
    }
    else {
      light_on = true;
      digitalWrite(D1, HIGH);
    }
  }
}

//// SETUP ////
void DMXBox::setup(){
  dmx.init(64);             // initialization for first 32 addresses by default
  delay(200);               // wait a while (not necessary)
  pinMode(D1, OUTPUT);
  BlickOneSecond();
}

void DMXBox::setup(int addresses){
  dmx.init(addresses);        // initialization for complete bus
  delay(200);               // wait a while (not necessary)
  pinMode(D1, OUTPUT);
  BlickOneSecond();
}

bool DMXBox::subscribe_to_topics(){
      bool error = false;
      char c_number[4];
      int num = _number;
      snprintf(c_number, 4, "%d", num);
      char topicBuffer[200] = "devices/DMXBox/";
      strcat(topicBuffer, c_number);
      strcat(topicBuffer, "/#");
      #ifdef debug
      Serial.print("Subscribed to: ");
      Serial.println(topicBuffer);
      #endif
      return _client.subscribe(topicBuffer);
}
