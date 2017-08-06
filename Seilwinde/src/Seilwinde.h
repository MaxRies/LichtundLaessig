/*
 * Autor: Simon Fritz
 *
 */

#ifndef Seilwinde_h
#define Seilwinde_h
#include "device.h"
#include <PubSubClient.h>

#define DROP_PIN D1
#define PULL_PIN D2
#define UP_BTN D8
#define DOWN_BTN D7

class Seilwinde : public Device
{
private:
  bool _handmodus = false;
  unsigned long _handmodus_ende;
  unsigned int _startTime; // in ms
  unsigned int _endTime;  // in ms
  bool droping;
  bool pulling;

  bool check_plaubsibility();
  void btn_loop();
  PubSubClient _client;
public:
  Seilwinde(PubSubClient client);
  void setup();
  void loop();
  void drop(int ms);
  void pull(int ms);
  void stop();
  bool subscribe_to_topics();
};

#endif
