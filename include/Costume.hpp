#include <Arduino.h>
#include <Config.hpp>
#include <FastLED.h>
#include <analogRGB.hpp>
#include <Arduino_FreeRTOS.h>
#include <Button.h>

CRGB dimm(CRGB color, uint8_t val) {
        return color.nscale8(val);
    }

void fill_snake2(CRGB *leds, uint16_t numLeds, uint16_t filledLeds) {
    for (unsigned int i = filledLeds; i < numLeds; i++) {
        leds[i] = leds[i%filledLeds];
    }
}

void fill_snake(CRGB *leds, uint16_t numLeds, uint16_t filledLeds) {
    for (unsigned int i = filledLeds; i< numLeds; i++) {
        if (int(i/filledLeds)&1)
            leds[i] = leds[filledLeds-i%filledLeds-1];
        else 
            leds[i] = leds[i%filledLeds];

    }
}

struct Flash {
    int pin;
    bool on = false;
    unsigned long time = 0;
};

class Signal
{
private:
    int startPixel = 0;
    public:
    CRGB ledStrip[NUM_LEDS_ROTATE];
    CRGB color;
public:
    Signal(){};
    ~Signal(){};
    void off(){
        for (int  i = 0; i < NUM_LEDS_ROTATE; i++)
        {
            ledStrip[i] = 0;
        }
        FastLED.show();
    };
    void rotate(CRGB color) {
        unsigned long xmillis = millis();
        for (int  i = 0; i < NUM_LEDS_ROTATE; i++)
        {   
            int val = 0xff - (ROTATES_TAIL_LENGTH * ((0xff / NUM_LEDS_ROTATE)) * (((int)(xmillis * ROTATE_HZ /1000 + i + startPixel) % NUM_LEDS_ROTATE)));
            val = val < 0 ? 0: val;
            ledStrip[i] = dimm(color, val);
        }
        this->startPixel++;
        FastLED.show();
    };
};

namespace Costume {
    Button btnSine = Button(PIN_BTN_SINE);
    Button btnSineAsync = Button(PIN_BTN_SINE_ASYNC);
    Button btnFlash = Button(PIN_BTN_FLASH);
    Button btnRotate = Button(PIN_BTN_ROTATE);
    Button btnPanic = Button(PIN_BTN_PANIC);
    Button swOn = Button(PIN_ON_SWITCH);
    TaskHandle_t flashTaskHandler = NULL;
    TaskHandle_t rotateTaskHandler = NULL;

    Signal frontSignal = Signal();
    analogRGB frontLED = analogRGB(PIN_FRONT_LED_R, PIN_FRONT_LED_G, PIN_FRONT_LED_B);
    CRGB frontRGB1[NUM_LEDS_FRONT_RGB_1];
    CRGB frontRGB2[NUM_LEDS_FRONT_RGB_2];

    Signal backSignal = Signal();
    analogRGB backLED = analogRGB(PIN_BACK_LED_R, PIN_BACK_LED_G, PIN_BACK_LED_B);
    
    Signal headSignal = Signal();
    analogRGB headLED = analogRGB(PIN_HEAD_LED_R, PIN_HEAD_LED_G, PIN_HEAD_LED_B);

    Flash flashes[5];
    uint32_t rotary = 0;
    const int rotaryLen = (sizeof(PINS_ROTARY)/sizeof(*PINS_ROTARY));
    const uint32_t rotaryMask = ((1 << (rotaryLen))-1);
    states_rotary rotaryStable = (states_rotary) 0; // stable state of rotary encoder 
    modes mode = (modes) 0; // stable current mode
    modes modeOld = (modes) 0; // prev mode
    bool modeChanged = false;
    bool oneBit(unsigned v) {return (v && !(v & (v - 1)));};
    double my_sin(double hz) {return (sin((millis()*hz*(2*PI/1000)))/2+0.5);}; // returns double between 0 and 1 depending on hz and current time
    CRGB color, color_gradient;
    uint8_t val;
    TaskHandle_t stateTaskHandler = NULL;
    unsigned long offSince = 0;


    void frontWhite(uint8_t val){
        analogWrite(PIN_FRONT_WHITE, val);
    };
    void headSpot(uint8_t val){
        analogWrite(PIN_HEAD_SPOT, val);
    };
    void setColor(CRGB color){
        for (int i = 0; i < NUM_LEDS_FRONT_RGB_1; i++) {
                frontRGB1[i] = color;
            }
        for (int i = 0; i < NUM_LEDS_FRONT_RGB_2; i++) {
            frontRGB2[i] = color;
        }
        FastLED.show();
        frontLED.setColor(color);
        backLED.setColor(color);
        headLED.setColor(color);
        analogWrite(LED_BUILTIN, color.getAverageLight());
    };
    void setColor(CRGB color, uint8_t val){
        setColor(color.nscale8(val));
    };
    void off(){
        setColor(CRGB::Black, 0);
        frontWhite(0);
        headSpot(0);
        frontSignal.off();
        backSignal.off();
        headSignal.off();
        for (int i = 0; i < 3; i++) {
            analogWrite(flashes[i].pin, 0);
        }
    };
 

    void update(void*) {
        unsigned long jumptime = 0;
        while (1) {
            if ((int) rotaryStable < 6) {
                color = colors[(int) rotaryStable];
            } else if (rotaryStable == states_rotary::fade) {
                uint8_t hue = ((millis()>>2)/FADE_DURATION)%256;
                hsv2rgb_rainbow(CHSV(hue, 255, 255), color);
                hsv2rgb_rainbow(CHSV(hue-COLOR_OFFSET, 255, 255), color_gradient);
                color.nscale8(MAX_BRIGHTNESS);
                color_gradient.nscale8(MAX_BRIGHTNESS);
                fill_gradient_RGB(frontRGB1, 12, color, color_gradient);
                fill_snake2(frontRGB1, NUM_LEDS_FRONT_RGB_1, 12);
                fill_gradient_RGB(frontRGB2, 12, color_gradient, color);
                fill_snake2(frontRGB2, NUM_LEDS_FRONT_RGB_2, 12);
                FastLED.show();
                frontLED.setColor(color);
                backLED.setColor(color);
                headLED.setColor(color);
                analogWrite(LED_BUILTIN, color.getAverageLight());
                vTaskDelay(0);
                continue;

            } else if (rotaryStable == states_rotary::jump) {
                unsigned long jumpmillis = millis();
                uint8_t hue = ((int)(jumpmillis/JUMP_MS))*73%256;
                int num = (int(jumpmillis*double(FALL_MS)/JUMP_MS)%256) > 60 ? 12 : (int(jumpmillis*double(FALL_MS)/JUMP_MS)%256)/5;
                if (jumpmillis - jumptime > JUMP_MS){
                    hsv2rgb_rainbow(CHSV(hue, 255, MAX_BRIGHTNESS), color);
                    frontLED.setColor(color);
                    backLED.setColor(color);
                    headLED.setColor(color);
                    analogWrite(LED_BUILTIN, color.getAverageLight());
                    jumptime = jumpmillis;
                    FastLED.clear();
                }
                FastLED.clear();
                num =int((jumpmillis - jumptime)/double(FALL_MS) * 11);
                num = num > 11 ? 11 : num;
                int start = (num - 5) < 0 ? 0 : num - 5;
                fill_gradient_RGB(frontRGB1,start, dimm(color, 50), num, color);
                fill_snake(frontRGB1, NUM_LEDS_FRONT_RGB_1, 12);
                fill_gradient_RGB(frontRGB2,11-start, dimm(color, 50),11 -num, color);
                fill_snake(frontRGB2, NUM_LEDS_FRONT_RGB_2, 12);
                FastLED.show(); 
                vTaskDelay(0);
                continue;

            } else if (rotaryStable == states_rotary::fade_async) {
                hsv2rgb_rainbow(CHSV((int)((millis()>>2)/FADE_ASYNC_DURATION*1.1)%256, 255, MAX_BRIGHTNESS), color) ;
                fill_rainbow(frontRGB1, NUM_LEDS_FRONT_RGB_1, (int)((millis()>>2)/FADE_ASYNC_DURATION*1.1)%256);
                hsv2rgb_rainbow(CHSV((int)((millis()>>2)/FADE_ASYNC_DURATION*0.97)%256, 255, MAX_BRIGHTNESS), color) ;
                fill_rainbow(frontRGB2, NUM_LEDS_FRONT_RGB_2, (int)((millis()>>2)/FADE_ASYNC_DURATION*1.05)%256);
                FastLED.show();
                hsv2rgb_rainbow(CHSV((int)((millis()>>2)/FADE_ASYNC_DURATION*0.9)%256, 255, 255), color) ;
                frontLED.setColor(color, MAX_BRIGHTNESS);
                hsv2rgb_rainbow(CHSV((int)((millis()>>2)/FADE_ASYNC_DURATION*0.93)%256, 255, 255), color) ;
                backLED.setColor(color, MAX_BRIGHTNESS);
                hsv2rgb_rainbow(CHSV((int)((millis()>>2)/FADE_ASYNC_DURATION)%256, 255, 255), color) ;
                headLED.setColor(color, MAX_BRIGHTNESS);
                analogWrite(LED_BUILTIN, dimm(color, 0xff*my_sin(FADE_ASYNC_DURATION)).getAverageLight());
                vTaskDelay(0);
                continue;
            }
            if (mode == modes::on) {
                val = MAX_BRIGHTNESS;
                setColor(color, val);
                
            } else if (mode == modes::sine) {
                val = MAX_BRIGHTNESS*my_sin(SINE_HZ);
                setColor(color, val);
            } else if (mode == modes::audio) {
                val = MAX_BRIGHTNESS;
                setColor(color, val);
            } else if (mode == modes::sine_async) {
                for (int i = 0; i < NUM_LEDS_FRONT_RGB_1; i++) {
                    frontRGB1[i] = dimm(color, MAX_BRIGHTNESS*my_sin(SINE_ASYNC_HZ*1.05));
                }
                for (int i = 0; i < NUM_LEDS_FRONT_RGB_2; i++) {
                    frontRGB2[i] = dimm(color, MAX_BRIGHTNESS*my_sin(SINE_ASYNC_HZ*1.1));
                }
                FastLED.show();
                frontLED.setColor(dimm(color, MAX_BRIGHTNESS*my_sin(SINE_ASYNC_HZ*0.97)));
                backLED.setColor(dimm(color, MAX_BRIGHTNESS*my_sin(SINE_ASYNC_HZ*0.93)));
                headLED.setColor(dimm(color, MAX_BRIGHTNESS*my_sin(SINE_ASYNC_HZ*0.90)));
                analogWrite(LED_BUILTIN, dimm(color.getAverageLight(), MAX_BRIGHTNESS*my_sin(SINE_ASYNC_HZ)));
            }
            // Serial.print("color: "); Serial.print(color); Serial.print(" , val: "); Serial.println(val);
            vTaskDelay(0);
        }
    };
    void rotateFun(void*) { 
        while(1) {
            headSignal.rotate(color);
            frontSignal.rotate(color);
            backSignal.rotate(color);
            vTaskDelay(1);
        }
    };
    void flashFun(void*) { 

        while (1) {
            for (int i = 0; i < 3; i++) {
                if (!flashes[i].on && (millis()-flashes[i].time > FLASH_OFF_MS)) {
                    flashes[i].on = true;
                    analogWrite(flashes[i].pin, 0xff);
                    flashes[i].time = millis();
                } else if (flashes[i].on && (millis()-flashes[i].time > FLASH_ON_MS)) {
                    flashes[i].on = false;
                    analogWrite(flashes[i].pin, 0);
                    flashes[i].time = millis();
                }
            }
            if (!flashes[3].on && (millis()-flashes[3].time > FRONT_WHITE_OFF_MS)) {
                    flashes[3].on = true;
                    analogWrite(flashes[3].pin, 0xff);
                    flashes[3].time = millis();
                } else if (flashes[3].on && (millis()-flashes[3].time > FRONT_WHITE_ON_MS)) {
                    flashes[3].on = false;
                    analogWrite(flashes[3].pin, 0);
                    flashes[3].time = millis();
                }
            Serial.println("in flash Fun"); 
            vTaskDelay(1);}
        };
    void panicFun(){ 
        vTaskSuspendAll(); 
        FastLED.setBrightness(MAX_BRIGHTNESS_PANIC);

        unsigned long time = millis();
        uint8_t frontHue1 = 0;
        uint8_t frontHue2 = 0;
        unsigned long xmillis = 0;
        unsigned long next = 500;
        while (millis() - time < 5000) {
            for (int i = 0; i < 5; i++) {
                /* if (!flashes[i].on && (millis()-flashes[i].time > FLASH_OFF_MS)) {
                    flashes[i].on = true;
                    analogWrite(flashes[i].pin, 0xff);
                    flashes[i].time = millis(); 
                } else */ if (flashes[i].on && (millis()-flashes[i].time > FLASH_ON_MS)) {
                    flashes[i].on = false;
                    analogWrite(flashes[i].pin, 0);
                    flashes[i].time = millis();
                }
            }
            frontHue1+=3;
            frontHue2+=2;
            FastLED.show();
            if (millis()- xmillis > next) {
                for (int i = 0; i < 5; i++) {
                    flashes[i].on = true;
                    analogWrite(flashes[i].pin, 0xff);
                    flashes[i].time = millis();
                }
                xmillis = millis();
                next = random(PANIC_MIN_MS, PANIC_MAX_MS);
                hsv2rgb_rainbow(CHSV(random(0xff), 255, MAX_BRIGHTNESS_PANIC), color);
                fill_solid(frontRGB1, NUM_LEDS_FRONT_RGB_1, color);
                hsv2rgb_rainbow(CHSV(random(0xff), 255, MAX_BRIGHTNESS_PANIC), color);
                fill_solid(frontRGB2, NUM_LEDS_FRONT_RGB_2, color);
                hsv2rgb_rainbow(CHSV(random(0xff), 255, MAX_BRIGHTNESS_PANIC), color);
                frontLED.setColor(color);
                hsv2rgb_rainbow(CHSV(random(0xff), 255, MAX_BRIGHTNESS_PANIC), color);
                backLED.setColor(color);
                hsv2rgb_rainbow(CHSV(random(0xff), 255, MAX_BRIGHTNESS_PANIC), color);
                headLED.setColor(color);
                hsv2rgb_rainbow(CHSV(random(0xff), 255, MAX_BRIGHTNESS_PANIC), color);
            }
            frontSignal.rotate(color);
            backSignal.rotate(color);

    
            
            Serial.println("in panic Fun"); vTaskDelay(10);
        }
        off();
        FastLED.setBrightness(MAX_BRIGHTNESS);
        xTaskResumeAll(); 
    };

    void input(void*){
        while(true) {
            // read buttons and rotary
            // read rotary
            for (int pin : PINS_ROTARY) {
                rotary = (rotary << 1) + !digitalRead(pin);
            }
            if (oneBit(rotary &= rotaryMask) && ((rotary & rotaryMask) != ((rotary >> rotaryLen)  & rotaryMask))) {
                int i = 0;
                while (!(rotary & 1)) {
                    rotary >>= 1;
                    i++;
                }
                rotaryStable = (states_rotary) i;
                offSince = millis();
            }; 

            // read buttons
            if (btnFlash.pressed()) {
                if (flashTaskHandler == NULL) {
                    xTaskCreate(flashFun, "flash", 1024, NULL, 2, &flashTaskHandler);
                } else {
                    vTaskDelete(flashTaskHandler);
                    flashTaskHandler = NULL;
                    for (auto flash : flashes) {
                        analogWrite(flash.pin, 0);
                    }
                }
            }
            if (btnRotate.pressed()) {
                if (rotateTaskHandler == NULL) {
                    xTaskCreate(rotateFun, "rotate", 1024, NULL, 2, &rotateTaskHandler);
                } else {
                    vTaskDelete(rotateTaskHandler);
                    rotateTaskHandler = NULL;
                    headSignal.off();
                    frontSignal.off();
                    backSignal.off();
                }
            } 
            if (btnSine.pressed()) {
                if (mode == modes::sine) {
                    mode = modes::on;
                } else {
                    mode = modes:: sine;
                }
            }
            if (btnSineAsync.pressed()) {
                if (mode == modes::sine_async) {
                    mode = modes::on;
                } else {
                    mode = modes::sine_async;
                }
            }
            if (btnPanic.pressed()) {
                panicFun();
            }

            if (swOn.pressed()) {
                mode = modes::off;
                if (stateTaskHandler != NULL) {
                    vTaskDelete(stateTaskHandler);
                    Serial.println("state task deleted");
                }
                if (rotateTaskHandler != NULL) {
                    vTaskDelete(rotateTaskHandler);
                }
                if (flashTaskHandler != NULL) {
                    vTaskDelete(flashTaskHandler);
                }
                off();
            } 
            
            if (swOn.released()) {
                mode = modes::on;
                xTaskCreate(update, "state", 2048, NULL, 1, &stateTaskHandler);
            } 
            // button-reading finished. now do stuff

            if (((mode == modes::sine) || (mode == modes::sine_async) || (rotateTaskHandler != NULL) || (flashTaskHandler != NULL)) && (mode != modes::off)) {
                headSpot(0xff);
            } else {
                headSpot(0);
            }

            if (modeChanged) {
                modeOld = mode;
                if (stateTaskHandler != NULL) {
                    vTaskDelete(stateTaskHandler);
                    Serial.println("old task deleted");
                }
                xTaskCreate(update, "state", 2048, NULL, 1, &stateTaskHandler);
                modeChanged = false;
                Serial.print("Mode: "); Serial.println(mode);
            }
            vTaskDelay(2);
        }
    };
    void initPins() {
        pinMode(PIN_ON_SWITCH, INPUT_PULLUP);
        pinMode(PIN_FRONT_STROBE, OUTPUT);
        pinMode(PIN_FRONT_ROTATE, OUTPUT);
        pinMode(PIN_FRONT_RGB_1, OUTPUT);
        pinMode(PIN_FRONT_RGB_2, OUTPUT);
        pinMode(PIN_FRONT_LED_R, OUTPUT);
        pinMode(PIN_FRONT_LED_G, OUTPUT);
        pinMode(PIN_FRONT_LED_B, OUTPUT);
        pinMode(PIN_FRONT_WHITE, OUTPUT);
        pinMode(PIN_BACK_STROBE, OUTPUT);
        pinMode(PIN_BACK_ROTATE, OUTPUT);
        pinMode(PIN_BACK_LED_R, OUTPUT);
        pinMode(PIN_BACK_LED_G, OUTPUT);
        pinMode(PIN_BACK_LED_B, OUTPUT);
        pinMode(PIN_HEAD_STROBE, OUTPUT);
        pinMode(PIN_HEAD_LED_R, OUTPUT);
        pinMode(PIN_HEAD_LED_G, OUTPUT);
        pinMode(PIN_HEAD_LED_B, OUTPUT);
        pinMode(PIN_HEAD_SPOT, OUTPUT);
        pinMode(PIN_SPEAKER_LEDS, OUTPUT);
        pinMode(LED_BUILTIN, OUTPUT);

        for (int pin : PINS_ROTARY) {
            pinMode(pin, INPUT_PULLUP);
        }
        for (int pin : PINS_BUTTONS) {
            pinMode(pin, INPUT_PULLUP);
        }
        pinMode(PIN_BTN_PANIC, INPUT_PULLUP);

        FastLED.addLeds<TM1804,PIN_FRONT_ROTATE,RGB>(frontSignal.ledStrip, NUM_LEDS_ROTATE).setCorrection(TypicalLEDStrip);
        FastLED.addLeds<TM1804,PIN_BACK_ROTATE,RGB>(backSignal.ledStrip, NUM_LEDS_ROTATE).setCorrection(TypicalLEDStrip);
        FastLED.addLeds<TM1804,PIN_FRONT_RGB_1,RGB>(frontRGB1, NUM_LEDS_FRONT_RGB_1).setCorrection(TypicalLEDStrip);
        FastLED.addLeds<TM1804,PIN_FRONT_RGB_2,RGB>(frontRGB2, NUM_LEDS_FRONT_RGB_2).setCorrection(TypicalLEDStrip);
        flashes[0].pin = PIN_FRONT_STROBE;
        flashes[1].pin = PIN_BACK_STROBE;
        flashes[2].pin = PIN_HEAD_STROBE;
        flashes[3].pin = PIN_FRONT_WHITE;
        flashes[4].pin = PIN_HEAD_SPOT;
        FastLED.setBrightness(MAX_BRIGHTNESS);
    };
} // namespace Costume