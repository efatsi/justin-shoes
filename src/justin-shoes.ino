#include <Adafruit_NeoPixel.h>

#define PRESSURE_PIN A0
#define LED_PIN      13

#define PIXELS    30
#define PIXEL_PIN 9

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.setBrightness(50);
  strip.begin();
  Serial.begin(9600);

  pinMode(PRESSURE_PIN, INPUT_PULLUP);
  pinMode(LED_PIN,      OUTPUT);
}

int   pressure          = 0;
bool  triggered         = false;
bool  lightOn           = false;
long  lightTimer        = millis();
long  rainbowStepTimer  = millis();
int   rainbowCount      = 0;
int   lightDuration     = 600;
int   recordedHigh      = 0;
int   recordedLow       = 100;
int   pressureThreshold = 40;
float sensitivity       = 0.4; // higher = easier to trigger

void loop() {
  determineState();
  display();
}

void determineState() {
  readPressure();
  setThreshold();
  determineLightState();
}

void readPressure() {
  pressure = analogRead(PRESSURE_PIN);

  if (pressure > recordedHigh) {
    recordedHigh = pressure;
  }
  if (pressure < recordedLow) {
    recordedLow = pressure;
  }
}

void setThreshold() {
  pressureThreshold = recordedLow + (recordedHigh - recordedLow) * (sensitivity);
}

long debounce = millis();
void determineLightState() {
  if (millis() - debounce < 50)
    return;

  if (triggered && pressure > pressureThreshold + 2) {
    debounce = millis();
    triggered = false;
  }
  if (!triggered && pressure < pressureThreshold - 2) {
    debounce = millis();
    triggered  = true;

    if (millis() - rainbowStepTimer < 400) {
      rainbowCount += 1;
      rainbowStepTimer = millis();
    } else {
      rainbowCount = 1;
      rainbowStepTimer = millis();
    }

    if (rainbowCount > 6) {
      rainbowCount = 0;
      rainbow(5);
    }

    if (!lightOn) {
      lightOn    = true;
      lightTimer = millis();
    }
  }

  if (lightOn && (millis() - lightTimer > lightDuration)) {
    lightOn = false;
    pixelsOff();
  }
}

long debugTimer = millis();
void display() {
  if (lightOn) {
    colorWipe(strip.Color(255, 90, 0));
    // colorWipe(strip.Color(0, 0, 255 ));
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  if (millis() - debugTimer > 200) {
    debugTimer = millis();
    Serial.printf("pressure: %d\n", pressure);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color) {
  long timePassed = millis() - lightTimer;
  float percentageComplete = (1.0 * timePassed) / lightDuration;

  if (percentageComplete < 0.5) {
    uint16_t furthestActivePixel = roundf(strip.numPixels() * percentageComplete * 2);

    for(uint16_t i=0; i<strip.numPixels(); i++) {
      if (i < furthestActivePixel) {
        strip.setPixelColor(i, color);
      } else {
        strip.setPixelColor(i, 0);
      }
    }
  } else {
    uint16_t leastActivePixel = roundf(strip.numPixels() * (percentageComplete - 0.5) * 2);

    for(uint16_t i=0; i<strip.numPixels(); i++) {
      if (i < leastActivePixel) {
        strip.setPixelColor(i, 0);
      } else {
        strip.setPixelColor(i, color);
      }
    }
  }

  strip.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (int times = 0; times < 3; times++) {
    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void pixelsOff() {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }

  strip.show();
}
