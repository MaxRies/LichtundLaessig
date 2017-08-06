#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Protocol.h"

// Connected periphery
#define potiPin A0
#define button1 D1
#define button2 D2
#define stroboled D4
#define nextled D3
// time for pattern in mins
#define patternTime 5
// time for color in mins
#define colorTime 14
// for debugging:
#define debug


// Current and temp values of periphery
int16_t poti_value = 0;
int16_t poti_temp = 0;
bool button1_pressed = false;
bool button1_temp = false;
bool button2_pressed = false;
bool button2_temp = false;
bool changed = false;
bool poti_changed = false;
bool buttons_changed = false;
bool strobobuttonleuchtet = false;

// current patterns
int current_base_pattern = 1;
int current_front_pattern = 2;
int current_strobe = 2;
int color_picker = 0;
int base_color = 128;
int front_color = 220;
int strobe_color = 128;
int main_dimmer = 255;
bool strobe_on = false;
unsigned long last_pattern_change = 0;
unsigned long last_color_change = 0;

// array for colors
int farben[90][2] = {{1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {2, 1},
  {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {2, 10}, {3, 1}, {3, 2},
  {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10}, {4, 1}, {4, 2}, {4, 3},
  {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9}, {4, 10}, {5, 1}, {5, 2}, {5, 3}, {5, 4},
  {5, 6}, {5, 7}, {5, 8}, {5, 9}, {5, 10}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5},
  {6, 7}, {6, 8}, {6, 9}, {6, 10}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6},
  {7, 8}, {7, 9}, {7, 10}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7},
  {8, 9}, {8, 10}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8},
  {9, 10}, {10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 5}, {10, 6}, {10, 7}, {10, 8}, {10, 9}
};


// Stuff for protocol
settingMessage setMesg;

// Timer for frequent updates
unsigned long oneSec = 0;
unsigned long strobotoggle = 0;

// WLAN credentials and status
char ssid[] = "test";     //  your network SSID (name)
char pass[] = "testtest";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiUDP udp_sender;
IPAddress localIP;
IPAddress broadcast;
const uint32_t port = 1103;

// Connecting to WiFi
void wifiSetup() {
  WiFi.begin(ssid, pass);
#ifdef debug
  Serial.print("Attempting to connect to WiFi");
#endif
  while (WiFi.status() != WL_CONNECTED) {
#ifdef debug
    Serial.print(".");
#endif
    digitalWrite(stroboled, HIGH);
    digitalWrite(nextled, HIGH);
    delay(500);
    digitalWrite(stroboled, LOW);
    digitalWrite(nextled, LOW);
    delay(500);
  }
#ifdef debug
  Serial.println("Connected!");
#endif
}

void setupBroadcast() {
  broadcast = WiFi.localIP();
  broadcast[3] = 255;
  if (udp_sender.begin(port) == 1) {
#ifdef debug
    Serial.printf("UDP Service Started at port %d", port);
#endif
  }
}

void nextPattern() {
  current_base_pattern++;
  if (current_base_pattern > 6) {
    current_base_pattern = 1;
    current_front_pattern++;
    if (current_front_pattern > 7) {
      current_front_pattern = 1;
    }
  }
  if ((current_front_pattern == 1) && (current_base_pattern == 1)) {
    current_base_pattern = 2;
  }
#ifdef debug
  Serial.print("front: ");
  Serial.print(current_front_pattern);
  Serial.print(" back: ");
  Serial.println(current_base_pattern);
#endif
  last_pattern_change = millis();
}

void lastPattern() {
  current_base_pattern--;
  if (current_base_pattern < 1) {
    current_base_pattern = 6;
    current_front_pattern--;
    if (current_front_pattern < 1) {
      current_front_pattern = 7;
    }
  }
}

void nextStrobe() {
  current_strobe++;
  if (current_strobe > 4) {
    current_strobe = 2;
  }
#ifdef debug
  Serial.print("Strobe mode: ");
  Serial.println(current_strobe);
#endif
}

boolean potiChanged() {
  poti_temp = analogRead(A0);
  if ((poti_temp > poti_value + 10) || (poti_temp < poti_value - 10)) {
    poti_value = poti_temp;
    return true;
  } else {
    return false;
  }
}

void setup() {
  // put your setup code here, to run once:
#ifdef debug
  Serial.begin(9600);
#endif
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(potiPin, INPUT);
  pinMode(nextled, OUTPUT);
  pinMode(stroboled, OUTPUT);
  wifiSetup();
  setupBroadcast();
  analogWrite(nextled, 25);
  analogWrite(stroboled, 25);
  oneSec = millis();
  last_color_change = millis();
  last_pattern_change = millis();
}

void loop() {
  delay(100);
  // put your main code here, to run repeatedly:
  int poti_temp = analogRead(A0);
  int button1_temp = digitalRead(D1);
  int button2_temp = digitalRead(D2);

  buttons_changed = false;
  changed = false;
  poti_changed = potiChanged();

  if (millis() > last_pattern_change + (patternTime * 60000)) {
    nextPattern();
    changed = true;
#ifdef debug
    Serial.println("automatic mode change.");
#endif
    analogWrite(nextled, 0);
    delay(200);
    analogWrite(nextled, 1023);
    delay(200);
    analogWrite(nextled, 0);
    delay(200);
    analogWrite(nextled, 1023);
  }

  if (millis() > last_color_change + colorTime * 60000) {
    color_picker++;
    if (color_picker > 89) {
      color_picker = 0;
    }
    front_color = floor(farben[color_picker][1] * 255 / 10);
    base_color = floor(farben[color_picker][2] * 255 / 10);
    changed = true;
    last_color_change = millis();
#ifdef debug
    Serial.print("color timer changed: ");
    Serial.print(color_picker);
    Serial.print(" ");
    Serial.print(front_color);
    Serial.print(" ");
    Serial.println(base_color);
#endif
  }

  /*if ((button2_temp == false) && strobe_on) {
    strobe_on = false;
    changed = true;
    }*/

  if (button1_temp != button1_pressed) {
    buttons_changed = true;
    button1_pressed = button1_temp;
  }
  if (button2_temp != button2_pressed) {
    buttons_changed = true;
    button2_pressed = button2_temp;
  }
  if (poti_changed) {
    buttons_changed = true;
  }

  // Hat sich was an den Buttons geändert?
  if (buttons_changed) {
    // Check which button is active
    if (button1_temp) {
      // Next button gedrückt!
      if (strobe_on) {
        changed = true;
        // Next gedrückt und Strobo an
        // also nächstes Strobe
        nextStrobe();
#ifdef debug
        Serial.println("next strobe");
#endif
      } else {
        // nur next gedrückt, jetzt poti auswerten!
        if (poti_changed) {
          // next gedrückt und poti geändert
          changed = true;
          color_picker = (int)(poti_temp * (89.0 / 1024.0));
          front_color = floor(farben[color_picker][0] * (255 / 10));
          base_color = floor(farben[color_picker][1] * (255 / 10));
#ifdef debug
          Serial.print("color poti changed: ");
          Serial.print(poti_temp);
          Serial.print(" ");
          Serial.print(color_picker);
          Serial.print(" ");
          Serial.print(front_color);
          Serial.print(" ");
          Serial.println(base_color);
#endif
        } else {
          changed = true;
          nextPattern();
#ifdef debug
          Serial.println("next pattern");
#endif
        }

      }
    }
    else if (button2_temp) {
      // nur Strobe gedrückt
      // also stroben
      // experimentieller code! start
      if (strobe_on) {
        strobe_on = false;
        changed = true;
#ifdef debug
        Serial.println("Strobe_on false gesetzt.");
#endif
      } else {
        strobe_on = true;
        changed = true;
#ifdef debug
        Serial.println("Strobe_on true gesetzt.");
#endif
      }
      // ende*/
      //strobe_on = true;
      if (poti_changed) {
        // experiment
        strobe_on = true;
        changed = true;
        // ende
        // Farbe anpassen und weiterstroben
        strobe_color = floor(poti_temp / 4);
        if (strobe_color < 35) {
          strobe_color = 35;
        }
#ifdef debug
        Serial.print("strobe color change: ");
        Serial.println(strobe_color);
#endif
      }
    } else if (poti_changed) {
      /*if (strobe_on) {
        // experiment
        strobe_on = true;
        // ende
        // Farbe anpassen und weiterstroben
        strobe_color = floor(poti_temp / 4);
        if (strobe_color < 35) {
          strobe_color = 35;
        }
#ifdef debug
        Serial.print("strobe color change: ");
        Serial.println(strobe_color);
#endif
      } else {*/
        changed = true;
        main_dimmer = floor(poti_temp / 4);
#ifdef debug
        Serial.print("maindimmer change: ");
        Serial.println(main_dimmer);
#endif
      //}
    }
  }

  // Nachricht bauen
  if (strobe_on) {
    setMesg.strobePattern = floor(current_strobe * (255 / 4)); // from 1 to 265
    setMesg.strobeColor = strobe_color;
    setMesg.strobeBrightness = 255;
#ifdef debug
    Serial.print("strobo: ");
    Serial.println(current_strobe);
#endif
  } else {
    setMesg.strobePattern = 1;
    setMesg.strobeColor = 1;
    setMesg.strobeBrightness = 1;
  }
  setMesg.basePattern = floor(current_base_pattern * (255 / 6));
  setMesg.frontPattern = floor(current_front_pattern  * (255 / 7));
  setMesg.mainDim = main_dimmer;
  setMesg.frontColor = front_color;
  setMesg.baseColor = base_color;
  setMesg.baseSpeed = 200;
  setMesg.frontSpeed = 70;
  setMesg.strobeSpeed = 100;
  setMesg.dutyCycle = 200;
  setMesg.frontBrightness = 255;
  setMesg.baseBrightness = 255;

  if (!setMesg.strobePattern) setMesg.strobePattern = 1;
  if (!setMesg.strobeColor) setMesg.strobeColor = 1;
  if (!setMesg.strobeBrightness) setMesg.strobeBrightness = 1;
  if (!setMesg.basePattern) setMesg.basePattern = 1;
  if (!setMesg.frontPattern) setMesg.frontPattern = 1;
  if (!setMesg.mainDim) setMesg.mainDim = 1;
  if (!setMesg.frontColor) setMesg.frontColor = 1;
  if (!setMesg.baseColor) setMesg.baseColor = 1;
  if (!setMesg.frontSpeed) setMesg.frontSpeed = 1;
  if (!setMesg.strobeSpeed) setMesg.strobeSpeed = 1;
  if (!setMesg.dutyCycle) setMesg.dutyCycle = 1;
  if (!setMesg.frontBrightness) setMesg.frontBrightness = 1;
  if (!setMesg.baseBrightness) setMesg.baseBrightness = 1;

  // build messages
  if (changed) {
    udp_sender.beginPacket(broadcast, port);
    udp_sender.write(setMesg.buffer, 15);
    udp_sender.endPacket();
#ifdef debug
    Serial.println("Send Package because change");
#endif
  }

  if (millis() - oneSec > 1000) {
    udp_sender.beginPacket(broadcast, port);
    udp_sender.write(setMesg.buffer, 15);
#ifdef debug
    for (int i = 0; i < setMesg.maxSize; i++) {
      Serial.print(" ");
      Serial.print(setMesg.buffer[i]);
    }

    Serial.println();
#endif
    udp_sender.endPacket();
#ifdef debug
    Serial.println("Send Package because timer");
#endif
    oneSec = millis();
  }

  if (button1_temp) {
    analogWrite(nextled, 1023);
  } else {
    analogWrite(nextled, 25);
  }

  if (button2_temp) {
    analogWrite(stroboled, 1023);
    strobotoggle = millis();
  } else if (strobe_on) {
    if (millis() > strobotoggle + 500) {
      if (strobobuttonleuchtet){
        analogWrite(stroboled, 25);
        strobobuttonleuchtet = false;
        strobotoggle = millis();
      } else {
        analogWrite(stroboled, 1023);
        strobobuttonleuchtet = true;
        strobotoggle = millis();
      }
    }
  } else {
    analogWrite(stroboled, 25);
  }

  if (WiFi.status() != WL_CONNECTED) {
    ESP.reset();
  }


}
