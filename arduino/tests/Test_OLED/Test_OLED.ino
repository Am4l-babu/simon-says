// =====================================================
//  TEST 3  -  SSD1306 128x64 OLED (I2C)
//
//  What it checks
//    - the display answers on I2C (0x3C)
//    - every pixel works (full-white, border, checkerboard)
//    - text sizes, shapes and inversion render correctly
//    - all the game's screens look right
//
//  If the screen stays blank:
//    - try OLED_ADDR 0x3D
//    - run the I2C scan printed in the Serial Monitor
// =====================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 4       // D2
#define SCL_PIN 5       // D1

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- helpers ----------

void i2cScan() {
  Serial.println(F("I2C scan:"));
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  device at 0x"));
      if (addr < 16) Serial.print('0');
      Serial.print(addr, HEX);
      if (addr == 0x3C || addr == 0x3D) Serial.print(F("  <- OLED"));
      if (addr >= 0x20 && addr <= 0x27) Serial.print(F("  <- PCF8574"));
      if (addr >= 0x38 && addr <= 0x3F && addr != 0x3C && addr != 0x3D)
        Serial.print(F("  <- PCF8574A"));
      Serial.println();
      found++;
    }
  }
  if (!found) Serial.println(F("  nothing found - check SDA/SCL/power"));
}

void caption(const char *text) {
  Serial.print(F("  screen: "));
  Serial.println(text);
}

// ---------- test screens ----------

void screenFillAndBorder() {
  caption("all pixels ON, then border");
  display.clearDisplay();
  display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
  display.display();
  delay(900);

  display.clearDisplay();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);      // all 4 edges must be visible
  display.drawLine(0, 0, 127, 63, SSD1306_WHITE);
  display.drawLine(127, 0, 0, 63, SSD1306_WHITE);
  display.display();
  delay(1200);
}

void screenCheckerboard() {
  caption("checkerboard (dead-pixel check)");
  for (int phase = 0; phase < 2; phase++) {
    display.clearDisplay();
    for (int y = 0; y < 64; y += 8)
      for (int x = 0; x < 128; x += 8)
        if (((x / 8) + (y / 8) + phase) % 2 == 0)
          display.fillRect(x, y, 8, 8, SSD1306_WHITE);
    display.display();
    delay(800);
  }
}

void screenTextSizes() {
  caption("text sizes 1-3");
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Size 1: Simon Says"));
  display.setTextSize(2);
  display.println(F("Size 2"));
  display.setTextSize(3);
  display.println(F("Size3"));
  display.display();
  delay(1800);
}

void screenShapes() {
  caption("shapes");
  display.clearDisplay();
  display.drawCircle(20, 32, 16, SSD1306_WHITE);
  display.fillCircle(64, 32, 16, SSD1306_WHITE);
  display.drawRoundRect(88, 16, 34, 32, 6, SSD1306_WHITE);
  display.fillTriangle(100, 44, 110, 20, 120, 44, SSD1306_WHITE);
  display.display();
  delay(1500);
}

void screenInvert() {
  caption("invert on / off");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(12, 24);
  display.println(F("INVERT"));
  display.display();
  for (int i = 0; i < 3; i++) {
    display.invertDisplay(true);
    delay(400);
    display.invertDisplay(false);
    delay(400);
  }
}

void screenContrast() {
  caption("contrast sweep");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 24);
  display.println(F("CONTRAST"));
  display.display();
  for (int c = 255; c >= 0; c -= 15) { display.ssd1306_command(SSD1306_SETCONTRAST); display.ssd1306_command(c); delay(40); }
  for (int c = 0; c <= 255; c += 15) { display.ssd1306_command(SSD1306_SETCONTRAST); display.ssd1306_command(c); delay(40); }
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0xCF);                        // library default
}

// ---------- the game's own screens ----------

void screenGameTitle() {
  caption("game: title");
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(8, 5);
  display.println(F("SIMON"));
  display.setCursor(8, 27);
  display.println(F("SAYS"));
  display.display();
  delay(1500);
}

void screenGameStart() {
  caption("game: start");
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 2);
  display.println(F("SIMON"));
  display.setCursor(10, 22);
  display.println(F("SAYS"));
  display.setTextSize(1);
  display.setCursor(20, 50);
  display.println(F("PRESS BUTTON"));
  display.display();
  delay(1500);
}

void screenGameWatch() {
  caption("game: watch");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("SIMON SAYS"));
  display.setCursor(80, 0);
  display.print(F("LVL 7"));
  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(5, 20);
  display.print(F("SCORE"));
  display.setCursor(78, 20);
  display.print(60);
  display.setTextSize(1);
  display.setCursor(20, 50);
  display.println(F("WATCH..."));
  display.display();
  delay(1500);
}

void screenGameYourTurn() {
  caption("game: your turn");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("LEVEL: 7"));
  display.setCursor(0, 15);
  display.print(F("SCORE: 60"));
  display.setTextSize(2);
  display.setCursor(15, 35);
  display.println(F("YOUR TURN"));
  display.display();
  delay(1500);
}

void screenGameLevelUp() {
  caption("game: level up");
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 8);
  display.println(F("LEVEL"));
  display.setCursor(50, 32);
  display.print(8);
  display.display();
  delay(1500);
}

void screenGameOver() {
  caption("game: game over");
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.println(F("GAME"));
  display.setCursor(10, 20);
  display.println(F("OVER"));
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print(F("Score: "));
  display.print(60);
  display.setCursor(70, 45);
  display.print(F("Best: "));
  display.print(120);
  display.display();
  delay(1800);
}

// ---------- Arduino ----------

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== SIMON SAYS : OLED TEST ==="));

  Wire.begin(SDA_PIN, SCL_PIN);
  i2cScan();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.print(F("[FAIL] SSD1306 not found at 0x"));
    Serial.println(OLED_ADDR, HEX);
    Serial.println(F("       Try address 0x3D, and check SDA/SCL + 3.3V/GND."));
    while (true) delay(1000);
  }

  Serial.print(F("[ OK ] SSD1306 found at 0x"));
  Serial.println(OLED_ADDR, HEX);
}

void loop() {
  Serial.println(F("--- hardware screens ---"));
  screenFillAndBorder();
  screenCheckerboard();
  screenTextSizes();
  screenShapes();
  screenInvert();
  screenContrast();

  Serial.println(F("--- game screens ---"));
  screenGameTitle();
  screenGameStart();
  screenGameWatch();
  screenGameYourTurn();
  screenGameLevelUp();
  screenGameOver();

  Serial.println(F("--- cycle complete, repeating ---"));
}
