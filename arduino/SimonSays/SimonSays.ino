#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// =====================================================
//                    PIN CONFIGURATION
// =====================================================

// ESP8266
#define SDA_PIN       4       // D2
#define SCL_PIN       5       // D1

#define WS2812_PIN    2       // D4
#define BUZZER_PIN   14       // D5

// =====================================================
//                    PCF8574
// =====================================================

#define PCF8574_ADDR 0x20

// PCF8574 outputs
#define LED1_BIT 0
#define LED2_BIT 1
#define LED3_BIT 2
#define LED4_BIT 3

// PCF8574 inputs
#define BUTTON1_BIT 4
#define BUTTON2_BIT 5
#define BUTTON3_BIT 6
#define BUTTON4_BIT 7

// =====================================================
//                       OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
//                    WS2812B
// =====================================================

// Change this to the number of LEDs you have
#define NUM_RGB_LEDS 24

Adafruit_NeoPixel pixels(
  NUM_RGB_LEDS,
  WS2812_PIN,
  NEO_GRB + NEO_KHZ800
);

// =====================================================
//                     GAME SETTINGS
// =====================================================

#define MAX_SEQUENCE 50

int sequence[MAX_SEQUENCE];

int sequenceLength = 1;
int playerPosition = 0;

int score = 0;
int highScore = 0;
int level = 1;

// Speed of displaying the sequence
int showTime = 600;

// Time allowed for player button
unsigned long playerTimeout = 5000;

// =====================================================
//                  RGB COLORS
// =====================================================

uint32_t colorRed;
uint32_t colorGreen;
uint32_t colorBlue;
uint32_t colorYellow;
uint32_t colorWhite;
uint32_t colorOff;

// =====================================================
//                  PCF8574 FUNCTIONS
// =====================================================

// Current state of PCF8574
uint8_t pcfState = 0xFF;

// Write complete byte to PCF8574
void pcfWrite(uint8_t value) {
  pcfState = value;

  Wire.beginTransmission(PCF8574_ADDR);
  Wire.write(value);
  Wire.endTransmission();
}

// Change one bit
void pcfSetBit(uint8_t bit, bool state) {

  if (state) {
    pcfState |= (1 << bit);
  } else {
    pcfState &= ~(1 << bit);
  }

  pcfWrite(pcfState);
}

// Read PCF8574
uint8_t pcfRead() {

  Wire.requestFrom(PCF8574_ADDR, 1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0xFF;
}

// =====================================================
//                    LED FUNCTIONS
// =====================================================

void allGameLEDsOff() {

  // PCF8574 outputs are active HIGH
  pcfSetBit(LED1_BIT, false);
  pcfSetBit(LED2_BIT, false);
  pcfSetBit(LED3_BIT, false);
  pcfSetBit(LED4_BIT, false);
}

void gameLED(int number, bool state) {

  if (number == 0) {
    pcfSetBit(LED1_BIT, state);
  }

  else if (number == 1) {
    pcfSetBit(LED2_BIT, state);
  }

  else if (number == 2) {
    pcfSetBit(LED3_BIT, state);
  }

  else if (number == 3) {
    pcfSetBit(LED4_BIT, state);
  }
}

// =====================================================
//                    BUZZER
// =====================================================

void beep(int frequency, int duration) {

  tone(BUZZER_PIN, frequency, duration);

  delay(duration);

  noTone(BUZZER_PIN);
}

// Different sound for each button
void buttonSound(int button) {

  int frequencies[4] = {
    262,   // C
    330,   // E
    392,   // G
    523    // C
  };

  beep(frequencies[button], 120);
}

// =====================================================
//                  RGB FUNCTIONS
// =====================================================

uint32_t getButtonColor(int button) {

  switch (button) {

    case 0:
      return colorRed;

    case 1:
      return colorGreen;

    case 2:
      return colorBlue;

    case 3:
      return colorYellow;
  }

  return colorWhite;
}

void rgbOff() {

  pixels.clear();
  pixels.show();
}

void rgbAll(uint32_t color) {

  for (int i = 0; i < NUM_RGB_LEDS; i++) {
    pixels.setPixelColor(i, color);
  }

  pixels.show();
}

void rgbButton(int button) {

  uint32_t color = getButtonColor(button);

  for (int i = 0; i < NUM_RGB_LEDS; i++) {
    pixels.setPixelColor(i, color);
  }

  pixels.show();
}

void rgbSuccess() {

  rgbAll(colorGreen);
  delay(150);

  rgbOff();
  delay(100);

  rgbAll(colorGreen);
  delay(150);

  rgbOff();
}

void rgbError() {

  for (int x = 0; x < 3; x++) {

    rgbAll(colorRed);
    delay(150);

    rgbOff();
    delay(150);
  }
}

void rainbowEffect() {

  for (int j = 0; j < 256; j += 8) {

    for (int i = 0; i < NUM_RGB_LEDS; i++) {

      int hue = (i * 256 / NUM_RGB_LEDS + j) & 255;

      pixels.setPixelColor(
        i,
        pixels.gamma32(
          pixels.ColorHSV(hue * 256)
        )
      );
    }

    pixels.show();
    delay(15);
  }

  rgbOff();
}

// =====================================================
//                    OLED FUNCTIONS
// =====================================================

void oledTitle() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(8, 5);
  display.println("SIMON");

  display.setCursor(8, 27);
  display.println("SAYS");

  display.display();
}

void oledStartScreen() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(10, 2);
  display.println("SIMON");

  display.setCursor(10, 22);
  display.println("SAYS");

  display.setTextSize(1);

  display.setCursor(20, 50);
  display.println("PRESS BUTTON");

  display.display();
}

void oledGameScreen() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("SIMON SAYS");

  display.setCursor(80, 0);
  display.print("LVL ");
  display.print(level);

  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(5, 20);
  display.print("SCORE");

  display.setCursor(78, 20);
  display.print(score);

  display.setTextSize(1);

  display.setCursor(20, 50);
  display.println("WATCH...");

  display.display();
}

void oledYourTurn() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("LEVEL: ");
  display.print(level);

  display.setCursor(0, 15);
  display.print("SCORE: ");
  display.print(score);

  display.setTextSize(2);

  display.setCursor(15, 35);
  display.println("YOUR TURN");

  display.display();
}

void oledGameOver() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(10, 0);
  display.println("GAME");

  display.setCursor(10, 20);
  display.println("OVER");

  display.setTextSize(1);

  display.setCursor(0, 45);
  display.print("Score: ");
  display.print(score);

  display.setCursor(70, 45);
  display.print("Best: ");
  display.print(highScore);

  display.display();
}

void oledLevelUp() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(20, 8);
  display.println("LEVEL");

  display.setCursor(50, 32);
  display.print(level);

  display.display();
}

// =====================================================
//                   BUTTON READING
// =====================================================

// Returns:
// 0 = button 1
// 1 = button 2
// 2 = button 3
// 3 = button 4
// -1 = no button

int readButton() {

  uint8_t state = pcfRead();

  // Buttons are active LOW
  if (!(state & (1 << BUTTON1_BIT))) {
    return 0;
  }

  if (!(state & (1 << BUTTON2_BIT))) {
    return 1;
  }

  if (!(state & (1 << BUTTON3_BIT))) {
    return 2;
  }

  if (!(state & (1 << BUTTON4_BIT))) {
    return 3;
  }

  return -1;
}

// Wait until button is released
void waitButtonRelease() {

  while (readButton() != -1) {
    delay(10);
  }
}

// =====================================================
//               WAIT FOR START BUTTON
// =====================================================

void waitForStart() {

  oledStartScreen();

  rgbOff();

  while (true) {

    int button = readButton();

    if (button != -1) {

      buttonSound(button);

      gameLED(button, true);
      rgbButton(button);

      delay(250);

      gameLED(button, false);
      rgbOff();

      waitButtonRelease();

      break;
    }

    delay(10);
  }
}

// =====================================================
//              GENERATE GAME SEQUENCE
// =====================================================

void generateFirstSequence() {

  sequenceLength = 1;

  for (int i = 0; i < MAX_SEQUENCE; i++) {

    sequence[i] = random(0, 4);
  }
}

// Add one random button
void addSequenceStep() {

  if (sequenceLength < MAX_SEQUENCE) {

    sequence[sequenceLength] = random(0, 4);

    sequenceLength++;
  }
}

// =====================================================
//                SHOW SIMON SEQUENCE
// =====================================================

void showSequence() {

  oledGameScreen();

  delay(700);

  for (int i = 0; i < sequenceLength; i++) {

    int button = sequence[i];

    // Turn on physical LED
    gameLED(button, true);

    // RGB color
    rgbButton(button);

    // Sound
    buttonSound(button);

    delay(showTime);

    // Turn off
    gameLED(button, false);

    rgbOff();

    delay(180);
  }
}

// =====================================================
//               GET PLAYER INPUT
// =====================================================

bool getPlayerSequence() {

  oledYourTurn();

  unsigned long startTime = millis();

  playerPosition = 0;

  while (playerPosition < sequenceLength) {

    if (millis() - startTime > playerTimeout) {

      return false;
    }

    int button = readButton();

    if (button != -1) {

      // Player pressed button
      gameLED(button, true);

      rgbButton(button);

      buttonSound(button);

      delay(120);

      gameLED(button, false);
      rgbOff();

      waitButtonRelease();

      // Check answer
      if (button != sequence[playerPosition]) {

        return false;
      }

      playerPosition++;

      // Reset timeout for next button
      startTime = millis();
    }

    delay(5);
  }

  return true;
}

// =====================================================
//                    SUCCESS
// =====================================================

void correctRound() {

  score += 10;

  if (score > highScore) {
    highScore = score;
  }

  rgbSuccess();

  beep(700, 100);

  delay(200);
}

// =====================================================
//                    GAME OVER
// =====================================================

void gameOver() {

  allGameLEDsOff();

  rgbError();

  beep(150, 300);
  delay(100);
  beep(100, 500);

  oledGameOver();

  delay(2500);

  rgbOff();

  // Wait for release
  waitButtonRelease();

  delay(500);
}

// =====================================================
//                START NEW GAME
// =====================================================

void startGame() {

  score = 0;

  level = 1;

  sequenceLength = 1;

  showTime = 600;

  playerTimeout = 5000;

  generateFirstSequence();

  oledLevelUp();

  rgbAll(colorWhite);

  delay(500);

  rgbOff();

  delay(500);
}

// =====================================================
//                        SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // PCF8574
  // All pins HIGH initially.
  // P0-P3 can act as outputs.
  // P4-P7 are inputs.
  pcfWrite(0xFF);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  noTone(BUZZER_PIN);

  // WS2812B
  pixels.begin();

  pixels.setBrightness(80);

  pixels.clear();

  pixels.show();

  // Colors
  colorRed =
    pixels.Color(255, 0, 0);

  colorGreen =
    pixels.Color(0, 255, 0);

  colorBlue =
    pixels.Color(0, 0, 255);

  colorYellow =
    pixels.Color(255, 180, 0);

  colorWhite =
    pixels.Color(255, 255, 255);

  colorOff =
    pixels.Color(0, 0, 0);

  // OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDR)) {

    Serial.println("OLED ERROR");

    while (true) {

      rgbAll(colorRed);
      delay(200);

      rgbOff();
      delay(200);
    }
  }

  // Random seed
  randomSeed(
    micros() +
    analogRead(A0)
  );

  allGameLEDsOff();

  rgbOff();

  oledTitle();

  delay(1500);

  oledStartScreen();
}

// =====================================================
//                        LOOP
// =====================================================

void loop() {

  // Wait for player to start
  waitForStart();

  // Start game
  startGame();

  while (true) {

    Serial.print("Level: ");
    Serial.print(level);

    Serial.print("  Sequence: ");

    for (int i = 0; i < sequenceLength; i++) {

      Serial.print(sequence[i]);

      Serial.print(" ");
    }

    Serial.println();

    // ---------------------------------------------
    // Show sequence
    // ---------------------------------------------

    showSequence();

    // ---------------------------------------------
    // Player repeats sequence
    // ---------------------------------------------

    bool correct = getPlayerSequence();

    // ---------------------------------------------
    // Wrong answer
    // ---------------------------------------------

    if (!correct) {

      gameOver();

      return;
    }

    // ---------------------------------------------
    // Correct
    // ---------------------------------------------

    correctRound();

    // ---------------------------------------------
    // Increase level
    // ---------------------------------------------

    level++;

    // Faster sequence
    if (showTime > 180) {

      showTime -= 30;
    }

    // Give slightly more time at higher levels
    if (playerTimeout < 10000) {

      playerTimeout += 150;
    }

    // Add new sequence item
    addSequenceStep();

    // Level-up animation
    oledLevelUp();

    rgbAll(colorGreen);

    beep(800, 80);
    delay(100);

    beep(1000, 80);

    delay(400);

    rgbOff();

    delay(300);
  }
}
