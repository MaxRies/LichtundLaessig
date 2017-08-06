/*
 * Autor: Simon Fritz
 * und Max.
 */
#include "Pattern_Strobo.h"
#include <PubSubClient.h>
#include <Bounce2.h>

//-----------
#define STROBO_DURATION 50 				// in ms
#define DURATION_PATTERN_LINE 20		// in ms
#define DURATION_PATTERN_EXPLOSION 50	// in ms
#define DURATION_PATTERN_ASIABLINK 30	// in ms
#define COUNT_PATTERN_LINE 10			// Anzahl Linien im Pattern


Pattern_Strobo::Pattern_Strobo(PubSubClient client)
  : Device ("Strobo"){
    this->_client = client;
}



void Pattern_Strobo::set_pattern_color(int patternColor){
  line.fg = colors(farben[patternColor][0]);
  line.bg = colors(farben[patternColor][1]);
}

int Pattern_Strobo::line_check(){
	if(line.pos > NUM_LED_OBEN){
		return 0;
	}
	else if (line.pos < 0)
	{
		return NUM_LED_OBEN;
	}
	else
	{
		return line.pos;
	}
}

// das selbe wie drueber nur mit beliebiger Anzahl leds -> fuer stereo-pattern
int Pattern_Strobo::line_check(int num_led){
	if(line.pos > num_led){
		return 0;
	}
	else if (line.pos < 0)
	{
		return num_led;
	}
	else
	{
		return line.pos;
	}
}

// fades all colors of LEDs to color2 in "steps"-steps
void Pattern_Strobo::fadeToColor(CRGB *LEDs, int numLEDs, CRGB color2, int divisor){

	for(int i = 0; i < numLEDs; i++)
	{
		LEDs[i].r = LEDs[i].r + (color2.r - LEDs[i].r)/divisor;
		LEDs[i].g = LEDs[i].g + (color2.g - LEDs[i].g)/divisor;
		LEDs[i].b = LEDs[i].b + (color2.b - LEDs[i].b)/divisor;
	}
}

void Pattern_Strobo::convert() {
  if (convert_out) {
  // gerade oder ungerade?
  int rest = NUM_LED_OUT % 4;
  // iterator fuer unten und oben
  int j = 0;
  for(int i = 0; i < NUM_LED_OUT; i = i+4) {
    out[i] = unten[j];
    out[i+1] = oben[j];
    out[i+2] = oben[j+1];
    out[i+3] = unten[j+1];
    j = j+2;
  }

  if(rest == 2) {
    out[NUM_LED_OUT-2] = unten[NUM_LED_OBEN-1];
    out[NUM_LED_OUT-1] = oben[NUM_LED_UNTEN-1];
  }
  convert_out = false;
  }
}

///////////////// Pattern_Strobo /////////////////

void Pattern_Strobo::led100(int color){
	switch(color)
	{
		case 0:// aus!
			digitalWrite(D1, HIGH);
			digitalWrite(D2, HIGH);
			digitalWrite(D3, HIGH);
			break;
		case 1:// Rot D3
			digitalWrite(D1, HIGH);
			digitalWrite(D2, HIGH);
			digitalWrite(D3, LOW);
			break;
		case 2:// Gruen D2
			digitalWrite(D1, HIGH);
			digitalWrite(D2, LOW);
			digitalWrite(D3, HIGH);
			break;
		case 3:// Blau D1
			digitalWrite(D1, LOW);
			digitalWrite(D2, HIGH);
			digitalWrite(D3, HIGH);
			break;
		case 4:// Rot, Gruen
			digitalWrite(D1, HIGH);
			digitalWrite(D2, LOW);
			digitalWrite(D3, LOW);
			break;
		case 5:// Rot, Blau
			digitalWrite(D1, LOW);
			digitalWrite(D2, HIGH);
			digitalWrite(D3, LOW);
			break;
		case 6:// Gruen, Blau
			digitalWrite(D1, LOW);
			digitalWrite(D2, LOW);
			digitalWrite(D3, HIGH);
			break;
		case 7:// Weiss
			digitalWrite(D1, LOW);
			digitalWrite(D2, LOW);
			digitalWrite(D3, LOW);
			break;
		default:// Weiss oder Aus???
			digitalWrite(D1, HIGH);
			digitalWrite(D2, HIGH);
			digitalWrite(D3, HIGH);
			break;
	}
}

// erzeugt ein Strobolicht
// duration in ms
// color = (1,7)
void Pattern_Strobo::pattern_led100strobo(int color, int duration){
  static bool on = false;
  if (on) {
    if (millis() > last_event + 10) {
      on = false;
      led100(0);
      last_event = millis();
    }
  }
  else {
    if (millis() > last_event + duration) {
      on = true;
      led100(color);
      last_event = millis();
    }
  }

}

// forward:   false=anticlockwise / true=clockwise
// inverse:   false=normal        / true=invertiert => dreht front und back-color um
// count:     Anzahl Linien im mono-Modus
// duration in ms
void Pattern_Strobo::pattern_line(bool forward, bool inverse, int count, int duration){
	if(last_event + duration < millis())
	{
		led100(0);
		last_event = millis();
		int currentPos = 0;
		if(count <= 0)
		{
			count = 1;
		}
		else if (count > NUM_LED_OBEN)
		{
			count = NUM_LED_OUT;
		}

		CRGB colorf = line.fg;
		CRGB colorb = line.bg;
		if(inverse)
		{
			colorf = line.bg;
			colorb = line.fg;
		}
		fill_solid(out, NUM_LED_UNTEN, colorb);
		for(int i = 0; i<NUM_LED_OBEN;i++)
		{
			oben[i] = colorb;
			unten[i] = colorb;
		}
		if(forward)
		{
			line.pos++;
			line.pos = line_check();
			currentPos = line.pos;
			for(int i = 0; i < count; i++)
			{
				oben[line.pos] = colorf;
				unten[line.pos] = colorf;
				// addiert neue Position
				line.pos = line.pos + NUM_LED_OBEN / count;
				if(line.pos > NUM_LED_OBEN)
				{
					line.pos = line.pos - NUM_LED_OBEN; // wenn Ueberlauf dann korrigeren
				}
			}
		}
		else
		{
			line.pos--;
			line.pos = line_check();
			currentPos = line.pos;
			for(int i = 0; i < count; i++)
			{
				oben[line.pos] = colorf;
				unten[line.pos] = colorf;
				// addiert neue Position
				line.pos = line.pos + NUM_LED_OBEN / count;
				if(line.pos > NUM_LED_OBEN)
				{
					line.pos = line.pos - NUM_LED_OBEN; // wenn Ueberlauf dann korrigeren
				}
			}
		}
		line.pos = currentPos;
	}
  convert_out = true;
}

// erzeugt zwei linien die gegenlaeufig sind
// inverse:   false=normal        / true=invertiert => dreht front und back-color um
void Pattern_Strobo::pattern_line_stereo(bool forward, bool inverse, bool endless, int duration){
  convert_out = true;
	if(last_event + duration < millis())
	{
		led100(0);
		last_event = millis();
		int numLEDS = NUM_LED_OBEN/2;
		if(endless)
		{
			numLEDS = NUM_LED_OBEN;
		}

		int currentPos1 = line_check(numLEDS); // konvertiert die aktuelle Position in 40LEDs komforme Positionen
		int currentPos2 = NUM_LED_OBEN - currentPos1;

		CRGB colorf = line.fg;
		CRGB colorb = line.bg;
		if(inverse)
		{
			colorf = line.bg;
			colorb = line.fg;
		}
		fill_solid(out, NUM_LED_UNTEN, colorb);
		for(int i = 0; i<NUM_LED_OBEN;i++)
		{
		  oben[i] = colorb;
		  unten[i] = colorb;
		}
		if(forward)
		{
			line.pos++;
			currentPos1 = line_check(numLEDS);
			currentPos2 = NUM_LED_OBEN - currentPos1;
			oben[currentPos1] = colorf;
			unten[currentPos1] = colorf;
			oben[currentPos2] = colorf;
			unten[currentPos2] = colorf;
		}
		else
		{
			line.pos--;
			currentPos1 = line_check(numLEDS);
			currentPos2 = NUM_LED_OBEN - currentPos1;
			oben[currentPos1] = colorf;
			unten[currentPos1] = colorf;
			oben[currentPos2] = colorf;
			unten[currentPos2] = colorf;
		}
		line.pos = currentPos1;
	}
}

// erzeugt zwei linien die gegenlaeufig sind und bei jedem Aufeinandertreffen gibt es einen Blitz
void Pattern_Strobo::pattern_line_stereo_explosion(bool forward, bool inverse, bool endless, int duration, int explosionColor){
  convert_out = true;
	pattern_line_stereo(forward, inverse, endless, duration);
	if(line.pos == NUM_LED_OBEN/2 || line.pos == 0)
	{
    if(exploding == false)
    {
      exploding = true;
      last_explosion=millis();
      led100(explosionColor);
    }
	}
	if(millis()>last_explosion + 5)
	{
		led100(0);
		exploding = false;
	}
}

// erzeugt alle drei leds eine neue Farbe. Die Farben oben und unten sind dabei um eine Position verschoben
// in Abhaengigkeit der duration blinkt das pattern
// rot ,  rot,  rot : gruen,gruen,gruen : blau, blau, blau
// blau, blau, blau : rot  ,  rot,  rot : gruen,gruen,gruen
void Pattern_Strobo::pattern_asiaBlink(bool forward, int duration){
  convert_out = true;
	if(last_event + duration < millis())
	{
		last_event = millis();
		if(asiaBlinking)
		{
			asiaBlinking = false;
			// kill all
			led100(0);
			fill_solid(oben, NUM_LED_OBEN, CRGB::Black);
			fill_solid(unten, NUM_LED_OBEN, CRGB::Black);
		}
		else
		{
			asiaBlinking=true;
			led100(0); // kill 100WLEd if not used

			if(forward)
			{
				line.pos++;
				line.pos = line_check();
			}
			else
			{
				line.pos--;
				line.pos = line_check();
			}

			CRGB colors[] = {CRGB::Blue, CRGB::Red, CRGB::Green};
			int countcolor1 = 0;
			int countcolor2 = 1;
			int countdrei = 0;
			for(int i = 0; i < NUM_LED_OBEN; i++)
			{
				oben[i] = colors[countcolor1];
				unten[i] = colors[countcolor2];
				countdrei++;
				if(countdrei == 3)
				{
					countdrei = 0;
					countcolor1++;
					countcolor2++;
					if(countcolor1 == 3)
					{
						countcolor1 = 0;
					}
					if(countcolor2 == 3)
					{
						countcolor2 = 0;
					}
				}
			}
		}
	}
}

// bei jedem blinken blitzt die 100w led 5ms auf
void Pattern_Strobo::pattern_asiaBlink_flash(bool forward, int duration){
  convert_out = true;
	if(last_event + duration < millis())
	{
		last_explosion = millis();
		led100(7);
	}
	if(last_explosion + 5 < millis())
	{
		led100(0);
	}
	pattern_asiaBlink(forward, duration); // setzt auch last_event zurueck
}

// erzeugt eine Explosions-Animation
void Pattern_Strobo::pattern_explosion(int duration){
  convert_out = true;
	if(last_event + duration < millis())
	{
		last_event = millis();
		// kill all
		led100(0);
		for(int i = 0; i < NUM_LED_OBEN; i++)
		{
			oben[i] = CRGB::Black;
			unten[i] = CRGB::Black;
		}
	}
	if(last_event + duration/6*5 < millis())// Step 6 der Explosion
	{
		// kill all
		led100(0);
		fill_solid(oben, NUM_LED_OBEN, CRGB::Black);
		fill_solid(unten, NUM_LED_OBEN, CRGB::Black);
	}
	else if(last_event + duration/6*4 < millis())// Step 5 der Explosion
	{
		led100(0); // 100wLED = aus
		fadeToColor(oben, NUM_LED_OBEN, CRGB::Red, 2);
		fadeToColor(unten, NUM_LED_OBEN, CRGB::Red, 2);
	}
	else if(last_event + duration/6*3 < millis())// Step 4 der Explosion
	{
		led100(1); // 100wLED = aus
		fadeToColor(oben, NUM_LED_OBEN, CRGB::Red, 2);
		fadeToColor(unten, NUM_LED_OBEN, CRGB::Red, 2);
	}
	else if(last_event + duration/6*2 < millis())// Step 3 der Explosion
	{
		led100(1); // 100wLED = Rot
		fill_solid(oben, NUM_LED_OBEN, CRGB::White);
		fadeToColor(unten, NUM_LED_OBEN, CRGB::Red, 2);
	}
	else if(last_event + duration/6*1 < millis())// Step 2 der Explosion
	{
		led100(7); // 100wLED = Weiss
		fill_solid(unten, NUM_LED_OBEN, CRGB::White);
	}
	else // Step 1 der Explosion
	{
		led100(7); // 100wLED = Weiss
	}
}

void Pattern_Strobo::pattern_rotor(bool forward, bool inverse, int duration, int count){
  convert_out = true;
	if(last_event + duration < millis())
	{
		led100(0);
		last_event = millis();
		int currentPos = 0;
		if(count <= 0)
		{
			count = 1;
		}
		else if (count > NUM_LED_OBEN)
		{
			count = NUM_LED_OUT;
		}

		CRGB colorf = line.fg;
		CRGB colorb = line.bg;
		if(inverse)
		{
			colorf = line.bg;
			colorb = line.fg;
		}
		if(forward)
		{
			line.pos++;
			line.pos = line_check();
			currentPos = line.pos;
			for(int i = 0; i < count; i++)
			{
				if(oben[line.pos] == colorf)
				{
					oben[line.pos] = colorb;
					unten[line.pos] = colorb;
				}
				else
				{
					oben[line.pos] = colorf;
					unten[line.pos] = colorf;
				}
				// addiert neue Position
				line.pos = line.pos + NUM_LED_OBEN / count;
				if(line.pos > NUM_LED_OBEN)
				{
					line.pos = line.pos - NUM_LED_OBEN; // wenn Ueberlauf dann korrigeren
				}
				line.pos = line_check();;
			}
		}
		else
		{
			line.pos--;
			line.pos = line_check();
			currentPos = line.pos;
			for(int i = 0; i < count; i++)
			{
				if(oben[line.pos] == colorf)
				{
					oben[line.pos] = colorb;
					unten[line.pos] = colorb;
				}
				else
				{
					oben[line.pos] = colorf;
					unten[line.pos] = colorf;
				}
				// addiert neue Position
				line.pos = line.pos + NUM_LED_OBEN / count;
				if(line.pos > NUM_LED_OBEN)
				{
					line.pos = line.pos - NUM_LED_OBEN; // wenn Ueberlauf dann korrigeren
				}
				line.pos = line_check();;
			}
		}
		line.pos = currentPos;
	}
}

void Pattern_Strobo::pulse(CHSV color, int interval){
  static bool falling = false;
  static int counter = 0;
  int now = millis();
  if (now > last_beat + interval) {
    last_beat = now;
    CRGB rgb_out;
    color.val = counter;
    hsv2rgb_rainbow( color, rgb_out);
    for(int i = 0; i < NUM_LED_OUT; i++) {
      out[i] = rgb_out;
    }
    if (falling) {
      counter = counter - 10;
    }
    else {
      counter = counter + 10;
    }
    if (counter > 254) {
      falling = true;
      counter = 255;
    }
    else if (counter < 1) {
      falling = false;
      counter = 0;
    }
  }
}

void Pattern_Strobo::random_stars(int chance) {
  // chance von 0 bis 255!
  for(int i=0; i < NUM_LED_OUT; i++) {
    if (random8() < chance) {
      out[i] = CRGB::White;
    }
    else {
      out[i].fadeToBlackBy(20);
    }
  }
}

///////////////// loop /////////////////

void Pattern_Strobo::loop(){
	//light_on
	//strobe_color
	//strobe_on
	//pattern
	//pattern_color
	if(stroboActive)
	{
		//int color, int duration
		// wenn justOnce = true; returned true nachdem sie einmal geflashed wurde
		// sonst laeuft es unendlich lange
		pattern_led100strobo(strobe_color, STROBO_DURATION);
	}
	else if(lightActive)
	{
		switch(patternNumber)
		{
			case 0: //pattern_line
      {
  			int choice = random8()/255*3; //Zufallszahl zwischen 0 und 3
  			switch(choice)
  			{
  				case 0:
  				//bool forward, bool inverse, int count, int duration
  				pattern_line(true, false, COUNT_PATTERN_LINE, DURATION_PATTERN_LINE);
  					break;
  				case 1:
  				pattern_line(false, false, COUNT_PATTERN_LINE, DURATION_PATTERN_LINE);
  					break;
  				case 2:
  				pattern_line(true, true, COUNT_PATTERN_LINE, DURATION_PATTERN_LINE);
  					break;
  				case 3:
  				pattern_line(false, true, COUNT_PATTERN_LINE, DURATION_PATTERN_LINE);
  					break;
  				default:
  				pattern_line(true, false, COUNT_PATTERN_LINE, DURATION_PATTERN_LINE);
  					break;
  			}
  				break;
      }
			case 1: //pattern_line_stereo
			{
        int choice1 = random8()/255*5; //Zufallszahl zwischen 0 und 5
			switch(choice1)
			{
				case 0:
				//bool forward, bool inverse, bool endless, int duration
				pattern_line_stereo(true, false, false, DURATION_PATTERN_LINE);
					break;
				case 1:
				pattern_line_stereo(false, false, false, DURATION_PATTERN_LINE);
					break;
				case 2:
				pattern_line_stereo(true, true, false, DURATION_PATTERN_LINE);
					break;
				case 3:
				pattern_line_stereo(false, true, false, DURATION_PATTERN_LINE);
					break;
				case 4:
				pattern_line_stereo(true, false, true, DURATION_PATTERN_LINE);
					break;
				case 5:
				pattern_line_stereo(true, true, true, DURATION_PATTERN_LINE);
					break;
				default:
				pattern_line_stereo(true, false, false, DURATION_PATTERN_LINE);
					break;
        }
			}
			case 2: //pattern_line_stereo_explosion
      {
      int choice2 = random8()/255*3; //Zufallszahl zwischen 0 und 3
			switch(choice2)
			{
				case 0:
				//bool forward, bool inverse, int count, int duration
				pattern_line_stereo_explosion(true, false, false, DURATION_PATTERN_LINE, 7);
					break;
				case 1:
				pattern_line_stereo_explosion(false, false, false, DURATION_PATTERN_LINE, 7);
					break;
				case 2:
				pattern_line_stereo_explosion(true, true, false, DURATION_PATTERN_LINE, 7);
					break;
				case 3:
				pattern_line_stereo_explosion(false, true, false, DURATION_PATTERN_LINE, 7);
					break;
				default:
				pattern_line_stereo_explosion(true, false, false, DURATION_PATTERN_LINE, 7);
					break;
			}
				break;
      }
			case 3: //pattern_explosion
			//int duration
			pattern_explosion(DURATION_PATTERN_EXPLOSION);
				break;
			case 4: //pattern_asiaBlink
			{int choice4 = random8()/255*1; //Zufallszahl zwischen 0 und 1
			switch(choice4)
			{
				case 0:
				//bool forward, int duration
				pattern_asiaBlink(true, DURATION_PATTERN_ASIABLINK);
					break;
				case 1:
				pattern_asiaBlink(false, DURATION_PATTERN_ASIABLINK);
					break;
				default:
				pattern_asiaBlink(true, DURATION_PATTERN_ASIABLINK);
					break;
			}
				break;
      }
			case 5:
			{ int choice5 = random8()/255*1; //Zufallszahl zwischen 0 und 1
			switch(choice5)
			{
				case 0:
				//bool forward, int duration
				pattern_asiaBlink_flash(true, DURATION_PATTERN_ASIABLINK);
					break;
				case 1:
				pattern_asiaBlink_flash(false, DURATION_PATTERN_ASIABLINK);
					break;
				default:
				pattern_asiaBlink_flash(true, DURATION_PATTERN_ASIABLINK);
					break;
			}
				break;
      }
			case 6: //pulse
			pulse(CHSV(line.fg.r, line.fg.g, line.fg.b), 40); // Mag Niko.
				break;
			case 7: //random_stars
			random_stars(2);
				break;
			default: // kill all
				led100(0);
				for(int i = 0; i < NUM_LED_OBEN; i++)
				{
					oben[i] = CRGB::Black;
					unten[i] = CRGB::Black;
				}
				break;
		}
	}
  else {
    led100(0);
    fill_solid(out, NUM_LED_OUT, CRGB::Black);
  }
  convert();
  FastLED.show();
}

//// SETUP ////

void Pattern_Strobo::setup(){
	pinMode(LED_PIN_1, OUTPUT);
	pinMode(LED_PIN_2, OUTPUT);
	FastLED.addLeds<WS2811, LED_PIN_1, BRG>(out, 0, NUM_LEDS_ON_PIN_1);
	FastLED.addLeds<WS2811, LED_PIN_2, BRG>(out, NUM_LEDS_ON_PIN_1, NUM_LEDS_ON_PIN_2);


	last_change = millis();
	pinMode(D1, OUTPUT);
	digitalWrite(D1, HIGH);
	pinMode(D2, OUTPUT);
	digitalWrite(D2, HIGH);
	pinMode(D3, OUTPUT);
	digitalWrite(D3, HIGH);
}

bool Pattern_Strobo::subscribe_to_topics(){
      bool error = false;
      char c_number[4];
      int num = _number;
      snprintf(c_number, 4, "%d", num);
      char topicBuffer[200] = "devices/Strobo/";
      strcat(topicBuffer, c_number);
      strcat(topicBuffer, "/#");
      #ifdef debug
      Serial.print("Subscribed to: ");
      Serial.println(topicBuffer);
      #endif
      return _client.subscribe(topicBuffer);
}


CRGB Pattern_Strobo::colors(int color) {
	CRGB ret;
	switch(color){
	case 1:
		ret = CRGB::White;
		break;

	case 2:
		ret = CRGB::Blue;
		break;

	case 3:
		ret = CRGB::Purple;
		break;

	case 4:
		ret = CRGB::RosyBrown;
		break;

	case 5:
		ret = CRGB::DarkOrchid;
		break;

	case 6:
		ret = CRGB::Tomato;
		break;

	case 7:
		ret = CRGB::Red;
		break;

	case 8:
		ret = CRGB::Red;
		break;

	case 9:
		ret = CRGB::Sienna;
		break;

	case 10:
		ret = CRGB::Fuchsia;
		break;
	default:
		ret = CRGB::Black;
		break;

	}

	return ret;

}

void Pattern_Strobo::login_light(int modus) {
  fill_solid(out, 160, colors(modus));
  FastLED.show();
  delay(500);
}
