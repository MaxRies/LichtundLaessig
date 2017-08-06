#include "knopfbox.h"

Knopfbox::Knopfbox(int number, MQTT_CALLBACK_SIGNATURE)
  : Device ("Knopfbox", number, callback){

}

void Knopfbox::pinSetup(){
  pinMode(VIBROPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT);
}

void Knopfbox::vibrate(int ms){
  this->_timeToVibe = ms;
  this->_lastVibe = millis();
  this->_vibrating = true;
}

void Knopfbox::vibe_worker(){
  if (_vibrating){
    if (millis() < _lastVibe + _timeToVibe){
      digitalWrite(VIBROPIN, HIGH);
    }
    else {
      digitalWrite(VIBROPIN, LOW);
      _vibrating = false;
    }
  }
}

void Knopfbox::lights_on(){
  if (_lit) {

  } else {
    digitalWrite(LEDPIN, HIGH);
    _lit = true;
  }
}

void Knopfbox::lights_off(){
  if (_lit) {
    digitalWrite(LEDPIN, LOW);
    _lit = false;
  } else {

  }
}

void Knopfbox::flash(int times){

}

void Knopfbox::flash_worker(){

}

void Knopfbox::loop(){
  vibe_worker();
  flash_worker();
}
