/*
 * Autor: Simon Fritz
 *
 */

#ifndef Pattern_Strobo_h
#define Pattern_Strobo_h
#include "device.h"
#include <PubSubClient.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"

#define NUM_LED_OUT 160
#define NUM_LED_OBEN 80       			// Diese beiden Zahlen sollten
#define NUM_LED_UNTEN 80      			// NUM_LED_OUT/2 entsprechen
#define LED_PIN_1 D4          			// 9 und 13 w�re RAW
#define LED_PIN_2 D5
#define NUM_LEDS_ON_PIN_1 84 			// Diese beiden Zahlen muessen
#define NUM_LEDS_ON_PIN_2 76  			// NUM_LED_OUT ergeben.

struct Line{
	int pos;
	int width;
	CRGB fg;  // Linienfarbe
	CRGB bg;  // Hintergrundfarbe

	Line(CRGB foreground, CRGB background, int width) {
	pos = 1;
	width = width;
	fg = foreground;
	bg = background;
	}
};


class Pattern_Strobo : public Device
{
private:
	//out = [1 a b 2 3  o u u o]
	//unten = [1 2 3 4 5 6 7 8 9 0]
	//oben =  [a b c d e f g h i j]

	Line line = Line(CRGB::Blue, CRGB::Black, 1);
	int farben[90][2];
	unsigned long last_beat = 0;
	unsigned int last_change = 0;
	unsigned long last_event = 0;
	unsigned long last_explosion = 0;
	int mode = 1;
	int bright_change = 0;
	bool convert_out = false;
	bool exploding = false;
	bool asiaBlinking = false;
	int lastPos = 0;
	CRGB out[NUM_LED_OUT];
	CRGB oben[NUM_LED_OBEN];
	CRGB unten[NUM_LED_UNTEN];
	CRGB foreground;
	CRGB background;
	//-------- Funktionen
	int line_check();
	int line_check(int num_led);
	void fadeToColor(CRGB *LEDs, int numLEDs, CRGB color2, int divisor);
	void convert();
	void led100(int color);
	void pattern_led100strobo(int color, int duration);
	void pattern_line(bool forward, bool inverse, int count, int duration);
	void pattern_line_stereo(bool forward, bool inverse, bool endless, int duration);
	void pattern_line_stereo_explosion(bool forward, bool inverse, bool endless, int duration, int explosionColor);
	void pattern_asiaBlink(bool forward, int duration);
	void pattern_asiaBlink_flash(bool forward, int duration);
	void pattern_explosion(int duration);
	void pattern_rotor(bool forward, bool inverse, int duration, int count);
	void pulse(CHSV color, int interval);
	void random_stars(int chance);
	CRGB colors(int number);
	PubSubClient _client;
public:
	Pattern_Strobo(PubSubClient client);
	//State-Flags
	bool stroboActive = false;	//strobe_on
	bool lightActive = false;	//light_on
	int patternNumber = 0;		//pattern
	int strobe_color = 0;		//strobe_color
	//-------- Funktionen
	void set_pattern_color(int patternColor);	//pattern_color
	void setup();
	void loop();
	bool subscribe_to_topics();
	void login_light(int modus);
};

#endif
