#include <Arduino.h>
#include "Button.h"

bool Button::fade_pending_g = false;
int Button::active_button_g = -1;
char Button::id_g = -1;

Button* button_ptr_g[4];

Button::Button(int led_pin_p, int button_pin_p, Indicator indicator_p) {
  id = ++id_g;
  led_pin = led_pin_p;
  button_pin = button_pin_p;
  pressed = false;
  was_pressed = false;
  brightness = 0;
  dir = 1;
  fading = false;
  indicator = indicator_p;
  button_ptr_g[id] = this;

  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

static void Button::fadeWait(int wait) {
  if (fade_pending_g) {
    delay(wait);
    fade_pending_g = false;
  }
}

void Button::setFade(bool starting) {
  fading = starting;
  if (!fading) {
    dir = -1;
  }
}

void Button::cancelFade() {
  fading = false;
  dir = -1;
}

void Button::updateFade() {
  if (!fading && ledCycleDone()) {
    return;
  }

  if (brightness >= 255 || brightness <= 0) {
    dir *= -1;
  }

  brightness = brightness + INCREMENT * dir;

  if (brightness > 255) {
    brightness = 255;
  }
  else if (brightness < 0) {
    brightness = 0;
  }

  analogWrite(led_pin, brightness);

  fade_pending_g = true;
}

bool Button::ledCycleDone () {
  return (!(bool)brightness);
}

void Button::updateButton() {
  pressed = !digitalRead(button_pin);
  if (pressed && !was_pressed) {
    setFade(!fading);
    if (fading) {
      active_button_g = id;
    }
    else {
      active_button_g = -1;
    }
  }
  was_pressed = pressed;
  if (active_button_g != id) {
    setFade(false);
  }
  updateFade();
}

Indicator Button::getIndicator() {
  return indicator;
}

static int Button::getActiveButton() {
  return Button::active_button_g;
}

static void Button::flashAll(int flash_count) {
  for (int i = 0; i < flash_count; i++) {
    for (int j = 0; j <= id_g; j++) {
      digitalWrite(button_ptr_g[j]->led_pin, HIGH);
    }
    delay(50);
    for (int j = 0; j <= id_g; j++) {
      digitalWrite(button_ptr_g[j]->led_pin, LOW);
    }
    delay(50);
  }
}

static void Button::cancelAll() {
  for (int i = 0; i <= id_g; i++) {
    button_ptr_g[i]->cancelFade();
  }
  active_button_g = -1;
}
