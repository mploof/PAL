#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <Arduino.h>

const int INCREMENT = 5;

typedef enum _indicator {
  FRONT_LEFT = 0xAA,
  FRONT_RIGHT = 0xBB,
  BACK_LEFT = 0xCC,
  BACK_RIGHT = 0xDD,
  OFF = 0xFF
} Indicator;

class Button {

  public:

    Button(int led_pin_p, int button_pin_p, Indicator indicator_p);

    void setFade(bool starting);
    void updateFade();
    bool ledCycleDone();
    void updateButton();
    Indicator getIndicator();
    void cancelFade();

    static void fadeWait(int wait);
    static int getActiveButton();
    static void flashAll(int flash_count);
    static void cancelAll();

  private:

    unsigned char id;
    int led_pin;
    int button_pin;
    bool pressed;
    bool was_pressed;
    int brightness;
    int dir;
    bool fading;
    Indicator indicator;

    static bool fade_pending_g;
    static int active_button_g;
    static char id_g;

};

extern Button* button_ptr_g[4];

#endif
