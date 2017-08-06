/*
Prototype class for LichtundLaessig peripherals.
Handles most of th ecommunication, except for the callback, which is
device specific.
*/
#include "device.h"

Device::Device(String type, int number, MQTT_CALLBACK_SIGNATURE){
  this->_number = number;
  this->_type = type;
  this->callback = callback;
}

String Device::getType(){
  return _type;
}

void Device::setNumber(int number){
  this->_number = number;
}

void Device::setupWifi(){
  setup_wifi();
}

void Device::onboardledblink(){
  for(int i = 0; i < 10; i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
