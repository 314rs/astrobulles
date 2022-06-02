#include <analogRGB.hpp>


analogRGB::analogRGB(int rPin, int gPin, int bPin)
{
    this->redPin = rPin;
    this->greenPin = gPin;
    this->bluePin = bPin;
    color = CRGB::Black;
}

analogRGB::~analogRGB() {}

void analogRGB::setColor(CRGB col) {
    this->color = col;
    analogWrite(redPin, color.red);
    analogWrite(greenPin, color.green);
    analogWrite(bluePin, color.blue);
}

void analogRGB::setColor(CRGB col, uint8_t brightness) {
    this->color = col.nscale8(brightness);
    analogWrite(redPin, color.red);
    analogWrite(greenPin, color.green);
    analogWrite(bluePin, color.blue);
}



