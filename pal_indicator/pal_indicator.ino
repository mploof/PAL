#include <FastLED.h>
#include <SoftwareSerial.h>

// How many leds in your strip?
#define NUM_LEDS 136
#define DATA_PIN 6

typedef enum _indicator {
  NONE = 0x00,
  INVALID = 0x01,
  FRONT_LEFT = 0xAA,
  FRONT_RIGHT = 0xBB,
  BACK_LEFT = 0xCC,
  BACK_RIGHT = 0xDD,
  OFF = 0xFF
} Indicator;

// Define the array of leds
CRGB leds[NUM_LEDS];

const int ARROW_COUNT = 4;

const int ARROW_LED_COUNT = 36;
int arrow[4][ARROW_LED_COUNT];
Indicator active_indicator = OFF;
bool fresh_start = true;

SoftwareSerial softSerial(10, 11); // RX, TX

void setup() {
  Serial.begin(57600);
  Serial.println("resetting");
  softSerial.begin(300);
  LEDS.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  LEDS.setBrightness(255);

  // Set the main arrow branch LEDs
  int start_val[4] = {60, 59, 60, 59};
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < ARROW_COUNT; j++) {
      arrow[j][i] = j % 2 ? start_val[j] - i : start_val[j] + i;
    }
  }

  // Set the arrow head LEDs
  for (int i = 0; i < ARROW_COUNT; i++) {
    int sign1 = i % 2 ? -1 : 1;
    int sign2 = i == 1 || i == 2 ? -1 : 1;
    int sign3 = i == 0 || i == 3 ? -1 : 1;
    int offset = i < 2 ? 9 : 32;

    for (int j = 0; j < 12; j++) {
      arrow[i][20 + j] = arrow[i][19] + offset * sign1 + j * sign2;
      if (j >= 8) {
        arrow[i][24 + j] = arrow[i][20 + j] + 12 * sign3;
      }
    }
    reverseLastN(arrow[i], ARROW_LED_COUNT, 4);
  }

  startupSplash();
  fadeOut();
}

void loop() {

  Indicator new_indicator = checkSerial();

  if (new_indicator != NONE) {

    // Fade out any active indicator
    // Check to see if there's an error
    if (new_indicator == INVALID) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
        FastLED.show();
      }
      Serial.println("Sending error code...");
      for (int i = 0; i < 6; i++) {
        softSerial.print('X');
      }
      active_indicator == OFF;
    }
    else {
      if (active_indicator != OFF) {
        Serial.println("Fading");
        fadeOut();
        Serial.println("Done fading");
      }
      active_indicator = new_indicator;
    }
    fresh_start = true;
  }

  updateAnimation(active_indicator, fresh_start);
  fresh_start = false;

}

void reverseLastN(int* list, int list_len, int rev_count) {
  int temp[rev_count];
  for (int i = 0; i < 4; i++) {
    temp[i] = list[list_len - rev_count + i];
  }
  for (int i = 0; i < rev_count; i++) {
    list[list_len - rev_count + i] = temp[rev_count - 1 - i];
  }
}

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  }
}

Indicator checkSerial() {
  Indicator ret = NONE;
  if (softSerial.available()) {
    Serial.println("*");
    delay(50);
    char input = 0;

    // Keep reading till we get to a good character
    while (softSerial.available() && !inputOk(input)) {
      input = softSerial.read();
      Serial.println(input);
    }

    // Dump the rest of the buffer
    while (softSerial.available()) {
      softSerial.read();
    }

    // If it's bad input, ignore it
    if (!inputOk(input)) {
      Serial.println("Bad input");
      ret = INVALID;
    }

    // Clear any remaining characters
    while (softSerial.available()) {
      softSerial.read();
    }

    if (ret != INVALID) {
      switch (input) {
        case 'A':
          ret = FRONT_LEFT;
          break;
        case 'B':
          ret = FRONT_RIGHT;
          break;
        case 'C':
          ret = BACK_LEFT;
          break;
        case 'D':
          ret = BACK_RIGHT;
          break;
        case 'F':
          ret = OFF;
          break;
        default:
          ret = 0;
          break;
      }
    }

    Serial.println("#");
  }
  return ret;
}

bool inputOk(int input) {
  if (input >= 'A' && input <= 'D' || input == 'F') {
    return true;
  }
  else {
    return false;
  }
}

void updateAnimation(Indicator indicator, bool fresh_start) {

  int arrow_val = -1;
  switch (indicator) {
    case OFF:
      return;
      break;
    case FRONT_LEFT:
      arrow_val = 0;
      break;
    case FRONT_RIGHT:
      arrow_val = 1;
      break;
    case BACK_LEFT:
      arrow_val = 2;
      break;
    case BACK_RIGHT:
      arrow_val = 3;
      break;
  }

  const int UPDATE_INTERVAL = 25;
  static long last_update = millis();

  if (millis() - last_update > UPDATE_INTERVAL) {

    // Pseudo for loop
    static int j = 0;
    if (fresh_start || j == ARROW_LED_COUNT - 4) {
      j = 0;
    }

    leds[arrow[arrow_val][j]] = CRGB::Red; //CHSV(hue++, 255, 255);

    if (j >= ARROW_LED_COUNT - 8) {
      leds[arrow[arrow_val][j + 4]] = CRGB::Red; //CHSV(hue++, 255, 255);
    }

    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    fadeall();
    fadeall();
    fadeall();

    j++;
    last_update = millis();
  }

}

void fadeOut() {
  for (int j = 0; j < 120; j++) {
    fadeall();
    FastLED.show();
    delay(2);
  }
}

void startupSplash() {
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
  Serial.print("x");

  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

void demoMode() {
  static uint8_t hue = 0;
  // First slide the led in one direction
  for (int i = 0; i < ARROW_COUNT; i++) {
    for (int k = 0; k < 4; k++) {
      for (int j = 0; j < ARROW_LED_COUNT - 4; j++) {
        char output[100];
        sprintf(output, "i: %d, j: %d, px: %d", i, j, arrow[i][j]);
        Serial.println(output);

        leds[arrow[i][j]] = CRGB::Red; //CHSV(hue++, 255, 255);

        if (j >= ARROW_LED_COUNT - 8) {
          leds[arrow[i][j + 4]] = CRGB::Red; //CHSV(hue++, 255, 255);
        }

        // Show the leds
        FastLED.show();
        // now that we've shown the leds, reset the i'th led to black
        // leds[i] = CRGB::Black;
        fadeall();
        fadeall();
        fadeall();
        fadeall();

        // Wait a little bit before we loop around and do it again
        delay(25);
      }
    }
    for (int j = 0; j < 120; j++) {
      fadeall();
      FastLED.show();
      delay(6);
    }
    delay(500);
  }
}
