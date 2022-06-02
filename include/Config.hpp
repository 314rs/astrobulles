#include <FastLED.h>
#pragma once

#ifndef CONFIG
#define CONFIG

#define PIN_FRONT_STROBE 22
#define PIN_FRONT_ROTATE 26
#define PIN_FRONT_RGB_1 28
#define NUM_LEDS_FRONT_RGB_1 60
#define NUM_LEDS_FRONT_RGB_2 60
#define PIN_FRONT_RGB_2 30
#define PIN_FRONT_LED_R 2
#define PIN_FRONT_LED_G 3
#define PIN_FRONT_LED_B 4
#define PIN_FRONT_WHITE 11

#define PIN_BACK_STROBE 42
#define PIN_BACK_ROTATE 44
#define PIN_BACK_LED_R 5
#define PIN_BACK_LED_G 6
#define PIN_BACK_LED_B 7

#define PIN_HEAD_STROBE 34
#define PIN_HEAD_LED_R 8
#define PIN_HEAD_LED_G 9
#define PIN_HEAD_LED_B 10
#define PIN_HEAD_SPOT 12

#define PIN_SPEAKER_LEDS 40


int PINS_ROTARY[] = {39,37,35,33,31,29,27,25,23};
int PINS_BUTTONS[] = {46,41,43,45};

#define PIN_BTN_SINE 46
#define PIN_BTN_SINE_ASYNC 41
#define PIN_BTN_FLASH 43
#define PIN_BTN_ROTATE 45
#define PIN_BTN_PANIC 47
#define PIN_ON_SWITCH 24

CRGB colors[] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Cyan, CRGB::Magenta, CRGB::Yellow};
enum states_rotary {red, green, blue, cyan, magenta, yellow, fade, jump, fade_async};
enum modes {on, sine, sine_async, off, audio};

#define FADE_DURATION 10 // time in seconds from red to red
#define JUMP_DURATION (0.7) //time in seconds to stay in same color

#define NUM_LEDS_ROTATE 24
#define STROBE_HZ 1
#define SINE_HZ (0.3)
#define SINE_ASYNC_HZ (0.3)
#define PANIC_HZ 4
#define PANIC_MIN_MS 10
#define PANIC_MAX_MS 200

#define FLASH_ON_MS 3 //on time in ms
#define FLASH_OFF_MS 300 //minimum off time in ms
#define FRONT_WHITE_ON_MS 3
#define FRONT_WHITE_OFF_MS 1000
#define ROTATES_TAIL_LENGTH 2 // Bigger = shorter

#define ROTATE_HZ (0.5)

#define PIN_AUDIO_IN A0

#endif //CONFIG