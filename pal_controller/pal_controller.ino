#include "Button.h"
#include <avr/wdt.h>

const int BUTTON_COUNT = 4;
int active_button = -1;
int last_active_button = -1;

Button button[BUTTON_COUNT] = {
  Button(3, 4, FRONT_LEFT),
  Button(5, 2, BACK_LEFT),
  Button(6, 8, BACK_RIGHT),
  Button(9, 7, FRONT_RIGHT)
};

void setup() {
  Serial.begin(300);
  //  Serial.println("Ready");
}

void loop() {

  if (isError()) {
    // If the command failed, indicate error on button panel
    Button::flashAll(10);
    Button::cancelAll();
  }

  // Get new info about the button, update the brightness
  for (int i = 0; i < BUTTON_COUNT; i++) {
    button[i].updateButton();
  }

  // Small delay for the fade increment
  Button::fadeWait(10);

  // See if the active button has changed
  active_button = Button::getActiveButton();
  if (active_button != last_active_button) {
    // If it has, send a new command to the indicator box
    bool success;
    if (active_button != -1) {
      success = sendCommand(button[active_button].getIndicator());
    }
    else {
      success = sendCommand(OFF);
    }
  }
  last_active_button = active_button;
}

bool isError() {
  if (Serial.available()) {
    delay(50);
    char input = 0;

    // Keep reading till we get to a good character
    while (Serial.available() && input != 'F') {
      input = Serial.read();
    }

    // Dump the rest of the buffer
    while (Serial.available()) {
      Serial.read();
    }

    if (input == 'X') {
      return true;
    }
  }
  return false;
}



bool sendCommand(Indicator indicator) {

  // Send the command
  char output;
  switch (indicator) {
    case OFF:
      output = 'F';
      break;
    case FRONT_LEFT:
      output = 'A';
      break;
    case FRONT_RIGHT:
      output = 'B';
      break;
    case BACK_LEFT:
      output = 'C';
      break;
    case BACK_RIGHT:
      output = 'D';
      break;
  }

  for (int i = 0; i < 6; i++) {
    Serial.print(indicator, HEX);
  }

}

void reboot() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
}
