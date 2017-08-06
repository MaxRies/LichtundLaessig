/*
Prototype class for LichtundLaessig peripherals.
Handles most of th ecommunication, except for the callback, which is
device specific.
*/
#include "device.h"

Device::Device(String type){
  this->_number = -1;  // erstmal error-ID setzen.
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
    digitalWrite(D4, LOW);
    delay(500);
    digitalWrite(D4, HIGH);
    delay(500);
}

void Device::onboardledblink(int n){
  for(int i = 0; i < n; i++){
    digitalWrite(D4, LOW);
    delay(500);
    digitalWrite(D4, HIGH);
    delay(500);
  }
}
