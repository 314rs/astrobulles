#include <FastLED.h>

class analogRGB
{
private:
    /* data */
    int redPin;
    int greenPin;
    int bluePin;
    CRGB color;

public:
    void setColor(CRGB col);
    void setColor(CRGB col, uint8_t brightness);
    analogRGB(int redPin, int greenPin, int bluePin);
    ~analogRGB();
};
