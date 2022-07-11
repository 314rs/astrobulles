#include <FastLED.h>
#pragma once

#ifndef CONFIG
#define CONFIG

#define PIN_FRONT_STROBE 22
#define PIN_FRONT_ROTATE 26
#define PIN_FRONT_RGB_1 28
#define NUM_LEDS_FRONT_RGB_1 60 // middle
#define NUM_LEDS_FRONT_RGB_2 36 // sides
#define PIN_FRONT_RGB_2 30
#define PIN_FRONT_LED_R 2
#define PIN_FRONT_LED_G 3
#define PIN_FRONT_LED_B 4
#define PIN_FRONT_WHITE 11

#define PIN_BACK_STROBE 42
#define PIN_BACK_ROTATE 44
#define PIN_BACK_LED_R 5
#define PIN_BACK_LED_G 7
#define PIN_BACK_LED_B 6

#define PIN_HEAD_STROBE 34
#define PIN_HEAD_LED_R 8
#define PIN_HEAD_LED_G 9
#define PIN_HEAD_LED_B 10
#define PIN_HEAD_SPOT 12

#define PIN_SPEAKER_LEDS 40
#define PIN_AUDIO_IN A0
#define NUM_LEDS_ROTATE 24

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
#define JUMP_MS 800 //time in ms to stay in same color
#define FALL_MS 400 //duration in ms leds fall down in jump mode


#define SINE_HZ (0.3) // frequency of color change in sine mode
#define SINE_ASYNC_HZ (0.3) //mean frequency of color change in sine async mode
#define PANIC_MIN_MS 10 // min ms until next color change in panic mode
#define PANIC_MAX_MS 200 // max ms until next color change in panic mode

#define FLASH_ON_MS 3 //on time in ms
#define FLASH_OFF_MS 300 //minimum off time in ms
#define FRONT_WHITE_ON_MS 3 // on-time of front white led strips (when flashes are enabled)
#define FRONT_WHITE_OFF_MS 1000 // off-time of front white led strips (when flashes are enabled)
#define ROTATES_TAIL_LENGTH 3 // bigger value = shorter tail
#define MAX_BRIGHTNESS 100 // from 0 to 255
#define MAX_BRIGHTNESS_PANIC 255 // from 0 to 255#
#define COLOR_OFFSET 127 // from 0 to 255 color offset when in fade mode

#define ROTATE_HZ (0.5)


#endif //CONFIG