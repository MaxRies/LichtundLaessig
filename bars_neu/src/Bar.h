#ifndef bar_h
#define bar_h
#include "device.h"
#include <PubSubClient.h>
//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define DEBUGMSG
#include <FastLED.h>
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "LedFunctions.h"
#include "Pattern.h"


#include "Protocol.h"


class Bar : public Device
{
private:
public:
  Bar(PubSubClient client);
  PubSubClient _client;
};

#endif
