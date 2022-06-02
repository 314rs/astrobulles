#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Config.hpp>
#include <Costume.hpp>

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(0));
    Costume::initPins();
    xTaskCreate(Costume::input, "update", 1024, NULL, 3, NULL);
}


// empty. Things are done in Tasks by freeRTOS
void loop() {}