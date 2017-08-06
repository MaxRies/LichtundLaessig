/*
 * Autor: Simon Fritz
 * und Max.
 */
#include "Seilwinde.h"
#include <PubSubClient.h>
#include <Bounce2.h>

Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();

Seilwinde::Seilwinde(PubSubClient client)
  : Device ("Seilwinde"){
    this->_client = client;
}

///////////////// Seilwinde /////////////////

void Seilwinde::drop(int ms){
  if (_handmodus == false) {
  this->_endTime = millis() + ms;
  this->droping = true;
  this->pulling = false;
}
}

void Seilwinde::pull(int ms){
  if (_handmodus == false) {
  this->_endTime = millis() + ms;
  this->droping = false;
  this->pulling = true;
}
}

void Seilwinde::stop(){
  this->_endTime = millis();
  this->droping = false;
  this->pulling = false;
}

bool Seilwinde::check_plaubsibility() {

  if (this->droping && this->pulling) {
    this->droping = false;
    this->pulling = false;
    return false;
  }
  else {
    return true;
  }
}

void Seilwinde::btn_loop() {
  debouncer1.update();
  debouncer2.update();
  int up_btn = debouncer1.read();
  int down_btn = debouncer2.read();

  if ((up_btn == 1) && (down_btn == 1)) {
    // do nothing bzw. Notaus!
    this->_handmodus = true;
    this->_handmodus_ende = millis() + 60000;
    this->pulling = false;
    this->droping = false;
  }
  else if ((up_btn == 1) && (down_btn == 0)) {
    this->_handmodus = true;
    this->_handmodus_ende = millis() + 60000;
    this->pulling = true;
    this->droping = false;
  } else if ((up_btn == 0) && (down_btn == 1)){
    this->_handmodus = true;
    this->_handmodus_ende = millis() + 60000;
    this->pulling = false;
    this->droping = true;
  }
  else {
    if (this->_handmodus) {
      if (millis() > this->_handmodus_ende) {
        this->_handmodus = false;
      }
      this->droping = false;
      this->pulling = false;
    }
    // do nothing so no Variables are changed.

  }
}

///////////////// loop /////////////////

void Seilwinde::loop(){
  btn_loop();
  if (check_plaubsibility()) {
  if(droping) {
    if (_handmodus == false) {
      if(millis() >= _endTime)
      {
        droping = false;
      }
      digitalWrite(DROP_PIN, LOW);
      digitalWrite(PULL_PIN, HIGH);
    }
    else if (_handmodus == true){
      digitalWrite(DROP_PIN, LOW);
      digitalWrite(PULL_PIN, HIGH);
  }
  }
  else if(pulling)
  {
    if (_handmodus == false) {
      if(millis() >= _endTime)
      {
        pulling = false;
      }
      digitalWrite(PULL_PIN, LOW);
      digitalWrite(DROP_PIN, HIGH);
    }
    else if (_handmodus == true){
        digitalWrite(PULL_PIN, LOW);
        digitalWrite(DROP_PIN, HIGH);
  }
}
  else
  {
    digitalWrite(DROP_PIN, HIGH);
    digitalWrite(PULL_PIN, HIGH);
  }
}
else {
  digitalWrite(DROP_PIN, HIGH);
  digitalWrite(PULL_PIN, HIGH);
}
}

//// SETUP ////
void Seilwinde::setup(){
    pinMode(DROP_PIN, OUTPUT);
    digitalWrite(DROP_PIN, HIGH); // Relais ist LOW-Aktiv!
    pinMode(PULL_PIN, OUTPUT);
    digitalWrite(PULL_PIN, HIGH);

    pinMode(UP_BTN, INPUT);
    pinMode(DOWN_BTN, INPUT);
    debouncer1.attach(UP_BTN);
    debouncer1.interval(5); // interval in ms
    debouncer2.attach(DOWN_BTN);
    debouncer2.interval(5); // interval in ms
}

bool Seilwinde::subscribe_to_topics(){
      bool error = false;
      char c_number[4];
      int num = _number;
      snprintf(c_number, 4, "%d", num);
      char topicBuffer[200] = "devices/Seilwinde/";
      strcat(topicBuffer, c_number);
      strcat(topicBuffer, "/#");
      #ifdef debug
      Serial.print("Subscribed to: ");
      Serial.println(topicBuffer);
      #endif
      return _client.subscribe(topicBuffer);
}
