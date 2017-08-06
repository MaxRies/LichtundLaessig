/*
Prototype class for LichtundLaessig peripherals.
Includes functions for setting up the MQTT Broker
*/
#include "device.h"

Device::Device(String type){
  this->_number = 0;  // erstmal error-ID setzen.
  this->_type = type;
}

String Device::getType(){
  return _type;
}

void Device::setNumber(int number){
  this->_number = number;
}

int Device::getNumber(){
  return _number;
}


void Device::onboardledblink(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}

void Device::onboardledblink(int n){
  for(int i = 0; i < n; i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
