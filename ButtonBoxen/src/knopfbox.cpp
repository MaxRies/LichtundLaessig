#include "knopfbox.h"
//#define debug

Knopfbox::Knopfbox(PubSubClient client)
  : Device ("StandardButtonBox"){
    _debouncer = Bounce();
    _client = client;
}

void Knopfbox::pinSetup(){
  pinMode(VIBROPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  _debouncer.attach(BUTTONPIN);
  _debouncer.interval(50);
}

void Knopfbox::vibe_on(){
  this->_vibe_permanently = true;
  this->_vibrate_times = false;
}

void Knopfbox::vibe_off(){
  this->_vibe_permanently = false;
  this->_vibrate_times = false;
}

void Knopfbox::vibe_times(int times) {
  this->_timesToVibe = times;
  this->_vibrate_times = true;
  this->_vibed_so_far = 0;
  this->_vibe_permanently = false;
  this->_lastVibe = 0;
}

void Knopfbox::vibe_once() {
  this->vibe_times(1);
}

void Knopfbox::vibe_worker(){
  if (_vibe_permanently) {
    digitalWrite(VIBROPIN, HIGH);
  }
  else if (_vibrate_times){
    if (_vibed_so_far < _timesToVibe) {
    if (millis() > _lastVibe + 1000){
      _lastVibe = millis();
      if (_vibe_on) {
        digitalWrite(VIBROPIN, LOW);
        _vibe_on = false;
      }
      else {
        digitalWrite(VIBROPIN, HIGH);
        _vibe_on = true;
        _vibed_so_far++;
        #ifdef debug
        Serial.println("VIBRIERT!");
        #endif
      }
    }
  }
  else {
    _vibrate_times = false;
    _vibed_so_far = 0;
    _timesToVibe = 0;
    _vibe_on = false;
  }
}
else {
  digitalWrite(VIBROPIN, LOW);
}
}


void Knopfbox::lights_on(){
  this->_flashing = false;
  this->_flash_permanently = false;
  this->_lit = true;
}

void Knopfbox::lights_off(){
    this->_flashing = false;
    this->_flash_permanently = false;
    this->_lit = false;
}

void Knopfbox::flash(int times_to_flash) {
  this->_flashing = true;
  this->_flash_permanently = false;
  this->_flashed = 0;
  this->_timesToFlash = times_to_flash;
  this->_lit = false;
}

void Knopfbox::flash_on() {
  this->_flash_permanently = true;
  this->_flashing = false;
  this->_lit = false;
}

void Knopfbox::flash_off() {
  this->_flash_permanently = false;
  this->_flashing = false;
}

void Knopfbox::flash_worker() {
  if (_flash_permanently) {
    unsigned long now = millis();
    if (now > _lastFlash + 300){
      if (_flashOn) {
        _flashOn = false;
        digitalWrite(LEDPIN, LOW);
        _lastFlash = now;
      }
      else {
        _flashOn = true;
        digitalWrite(LEDPIN, HIGH);
        _lastFlash = now;
      }
    }
  }
  else if (_flashing) {
    if (_flashed < _timesToFlash) {
      unsigned long now = millis();
      if (now > _lastFlash + 300){
        if (_flashOn) {
          _flashOn = false;
          digitalWrite(LEDPIN, LOW);
          _lastFlash = now;
          _flashed++;
        }
        else {
          _flashOn = true;
          digitalWrite(LEDPIN, HIGH);
          _lastFlash = now;
        }
      }
    }
  } else if (_lit) {
    digitalWrite(LEDPIN, HIGH);
  }
  else {
    digitalWrite(LEDPIN, LOW);
  }
}

void Knopfbox::beep_on() {
  digitalWrite(BEEPPIN, HIGH);
  this->_beeping = true;
  this->_beeping_n_times = false;
}

void Knopfbox::beep_off() {
  digitalWrite(BEEPPIN, LOW);
  this->_beeping = false;
  this->_beeping_n_times = false;
}

void Knopfbox::beep_times(int times){
  this->_times_to_beep = times;
  this->_beeping = false;
  this->_beeping_n_times = true;
  this->_beeps_so_far = 0;
}

void Knopfbox::beep_once() {
  beep_times(1);
}

void Knopfbox::beep_worker() {
  // drei Möglichkeiten: beep dauernd an
  // beep in intervallen
  // beep aus
  if (_beeping) {
    digitalWrite(BEEPPIN, HIGH);
  }
  else if (_beeping_n_times) {
    if (_beeps_so_far < _times_to_beep) {
      if (millis() > _last_beep + 500) {
        _last_beep = millis();
      // beep on off routine
      if (_beep_on) {
        digitalWrite(BEEPPIN, LOW);
        _beep_on = false;
        _beeps_so_far++;
      }
      else {
        digitalWrite(BEEPPIN, HIGH);
        _beep_on = true;
      }
    }
  }
    else {
      // nicht mehr beep_n_times
      _beeping_n_times = false;
      _times_to_beep = 0;
      _beeps_so_far = 0;
    }
  }
  else {
    digitalWrite(BEEPPIN, LOW);
  }
}

void Knopfbox::button_worker() {
  if (_debouncer.update() == 1) {
  int value = _debouncer.read();
  if (value == LOW) {
    _buttonPressed = false;
    char c_number[4];
    snprintf(c_number, 4, "%d", this->getNumber());
    char topicBuffer[200] = "devices/StandardButtonBox/";
    strcat(topicBuffer, c_number);
    strcat(topicBuffer, "/button");
    _client.publish(topicBuffer, "0");
    #ifdef debug
    Serial.println("Button released!");
    #endif
  }
  else if (value == HIGH) {
    _buttonPressed = true;
    char c_number[4];
    snprintf(c_number, 4, "%d", this->getNumber());
    char topicBuffer[200] = "devices/StandardButtonBox/";
    strcat(topicBuffer, c_number);
    strcat(topicBuffer, "/button");
    _client.publish(topicBuffer, "1");
    #ifdef debug
    Serial.println("Button pressed!");
    #endif
  }
  }
}

bool Knopfbox::buttonPressed() {
  return this->_buttonPressed;
}

bool Knopfbox::subscribe_to_topics() {
    #ifdef debug
    Serial.println("Entering subscribe to topics");
    #endif
    bool error = false;
    char c_number[4];
    int num = _number;
    snprintf(c_number, 4, "%d", num);
    char topicBuffer[200] = "devices/StandardButtonBox/";
    strcat(topicBuffer, c_number);
    strcat(topicBuffer, "/#");
    #ifdef debug
    Serial.print("Subscribed to: ");
    Serial.println(topicBuffer);
    #endif
    return _client.subscribe(topicBuffer, 1);
}

void Knopfbox::loop(){
  vibe_worker();
  flash_worker();
  beep_worker();
  button_worker();
}
