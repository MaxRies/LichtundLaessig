#ifndef knopfbox_h
#define knopfbox_h
#include <device.h>
#include <PubSubClient.h>
#include <Bounce2.h>

#define VIBROPIN D2
#define LEDPIN D6
#define BUTTONPIN D8
#define BEEPPIN D5

class Knopfbox : public Device
{
private:
  // Vibe Stuff
  bool _vibrate_times = false;
  bool _vibe_permanently = false;
  bool _vibe_on = false;
  unsigned int _timesToVibe;
  unsigned int _vibed_so_far = 0;
  unsigned int _lastVibe;
  void vibe_worker();
  // Light Stuff
  unsigned int _lastFlash;
  bool _flashing = false;
  int _timesToFlash;
  int _flashed;
  bool _lit = false;
  bool _flashOn = false;
  bool _flash_permanently = false;
  void flash_worker();
  // Button Stuff
  void button_worker();
  Bounce _debouncer;
  bool _buttonPressed = false;
  // Beep Stuff
  bool _beeping = false;
  bool _beeping_n_times = false;
  bool _beep_on = false;
  unsigned int _last_beep;
  unsigned int _times_to_beep;
  unsigned int _beeps_so_far;
  void beep_worker();
  // Communication Stuff
  PubSubClient _client;
public:
  Knopfbox(PubSubClient client);
  // Button stuff
  bool buttonPressed();
  // Vibe Stuff
  void vibe_on();
  void vibe_off();
  void vibe_times(int times);
  void vibe_once();
  // Beep stuff
  void beep_on();
  void beep_off();
  void beep_times(int times);
  void beep_once();
  // Light stuff
  void lights_on();
  void lights_off();
  void flash(int times_to_flash);
  void flash_on();
  void flash_off();
  // General stuff
  bool subscribe_to_topics();
  void pinSetup();
  void loop();
};

#endif
