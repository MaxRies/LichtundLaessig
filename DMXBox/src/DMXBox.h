/*
 * Autor: Simon Fritz
 *
 */

#ifndef DMXBox_h
#define DMXBox_h
#include "device.h"
#include <PubSubClient.h>

#define DROP_PIN D1
#define PULL_PIN D2
#define UP_BTN D8
#define DOWN_BTN D7

  class DMXBox : public Device
  {
  private:
    int Index;
    int channel;
    int value;
    bool light_on = false;
    void BlickOneSecond();
    unsigned int last_light;
    PubSubClient _client;
  public:
    DMXBox(PubSubClient client);
    void setup();
    void setup(int addresses);
    void loop();
    bool subscribe_to_topics();
    void update(String msg);
    void kill();
  };

#endif
