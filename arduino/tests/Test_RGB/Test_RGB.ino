// =====================================================
//  TEST 2  -  WS2812B RGB LED RING / STRIP
//
//  What it checks
//    - data line on D4 (GPIO2) works
//    - colour order is GRB (R,G,B must appear as R,G,B)
//    - every pixel lights (no dead LEDs along the chain)
//    - the exact 4 game colours look right
//
//  Cycle: solid colours -> game colours -> one-by-one walk
//         -> chase -> rainbow -> brightness fade -> repeat
//
//  Tip: if RED shows up as GREEN, change NEO_GRB to NEO_RGB
// =====================================================

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define WS2812_PIN    2       // D4
#define NUM_RGB_LEDS  24      // <- set this to YOUR LED count

Adafruit_NeoPixel pixels(NUM_RGB_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// Same colours the game uses
uint32_t colorRed, colorGreen, colorBlue, colorYellow, colorWhite;

void fill(uint32_t color) {
  for (int i = 0; i < NUM_RGB_LEDS; i++) pixels.setPixelColor(i, color);
  pixels.show();
}

void off() {
  pixels.clear();
  pixels.show();
}

void solid(const char *name, uint32_t color, int ms = 900) {
  Serial.print(F("  solid: "));
  Serial.println(name);
  fill(color);
  delay(ms);
  off();
  delay(200);
}

void pixelWalk(uint32_t color) {
  Serial.println(F("  pixel walk (every LED, in order)"));
  for (int i = 0; i < NUM_RGB_LEDS; i++) {
    pixels.clear();
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(60);
  }
  off();
}

void chase(uint32_t color, int laps) {
  Serial.println(F("  chase"));
  for (int lap = 0; lap < laps; lap++) {
    for (int i = 0; i < NUM_RGB_LEDS; i++) {
      pixels.clear();
      for (int t = 0; t < 4; t++) {                 // 4-pixel tail
        int p = (i - t + NUM_RGB_LEDS) % NUM_RGB_LEDS;
        uint8_t fade = 255 >> t;
        pixels.setPixelColor(p, pixels.Color(
          ((color >> 16) & 0xFF) * fade / 255,
          ((color >> 8)  & 0xFF) * fade / 255,
          ( color        & 0xFF) * fade / 255));
      }
      pixels.show();
      delay(35);
    }
  }
  off();
}

void rainbow() {
  Serial.println(F("  rainbow"));
  for (int j = 0; j < 256 * 2; j += 4) {
    for (int i = 0; i < NUM_RGB_LEDS; i++) {
      int hue = (i * 256 / NUM_RGB_LEDS + j) & 255;
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(hue * 256)));
    }
    pixels.show();
    delay(15);
  }
  off();
}

void brightnessFade() {
  Serial.println(F("  brightness fade (white)"));
  for (int b = 0; b <= 255; b += 5) {
    pixels.setBrightness(b);
    fill(colorWhite);
    delay(12);
  }
  for (int b = 255; b >= 0; b -= 5) {
    pixels.setBrightness(b);
    fill(colorWhite);
    delay(12);
  }
  pixels.setBrightness(80);    // back to the game's brightness
  off();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== SIMON SAYS : RGB LED TEST ==="));
  Serial.print(F("LEDs configured: "));
  Serial.println(NUM_RGB_LEDS);

  pixels.begin();
  pixels.setBrightness(80);          // same as the game
  pixels.clear();
  pixels.show();

  colorRed    = pixels.Color(255, 0, 0);
  colorGreen  = pixels.Color(0, 255, 0);
  colorBlue   = pixels.Color(0, 0, 255);
  colorYellow = pixels.Color(255, 180, 0);
  colorWhite  = pixels.Color(255, 255, 255);
}

void loop() {
  Serial.println(F("[1] Solid colours - expect RED, GREEN, BLUE, WHITE"));
  solid("RED",   colorRed);
  solid("GREEN", colorGreen);
  solid("BLUE",  colorBlue);
  solid("WHITE", colorWhite);

  Serial.println(F("[2] Game colours - expect RED, GREEN, BLUE, YELLOW"));
  solid("Button 1 = RED",    colorRed,    600);
  solid("Button 2 = GREEN",  colorGreen,  600);
  solid("Button 3 = BLUE",   colorBlue,   600);
  solid("Button 4 = YELLOW", colorYellow, 600);

  Serial.println(F("[3] Pixel walk - a single dot should visit every LED"));
  pixelWalk(colorWhite);

  Serial.println(F("[4] Chase"));
  chase(colorBlue, 2);

  Serial.println(F("[5] Rainbow"));
  rainbow();

  Serial.println(F("[6] Brightness"));
  brightnessFade();

  Serial.println(F("--- cycle complete, repeating ---"));
  delay(800);
}
