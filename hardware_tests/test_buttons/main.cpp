// =====================================================
//  TEST 1  -  PUSH BUTTONS  (via PCF8574 I2C expander)
//
//  What it checks
//    - the PCF8574 answers on I2C (0x20)
//    - all 4 buttons read correctly (active LOW)
//    - each button lights its game LED (P0-P3) and plays its note
//
//  How to use
//    Open the Serial Monitor at 115200 baud and press the buttons.
//    Every press / release prints one line, plus the raw state of the
//    four button pins (P7..P4, a 0 means "pressed").
// =====================================================

#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN      4    // D2
#define SCL_PIN      5    // D1
#define BUZZER_PIN  14    // D5

#define PCF8574_ADDR 0x20

// P0-P3 = LEDs (outputs) | P4-P7 = buttons (inputs, active LOW)
const uint8_t LED_BIT[4]    = {0, 1, 2, 3};
const uint8_t BUTTON_BIT[4] = {4, 5, 6, 7};
const int     NOTE[4]       = {262, 330, 392, 523};   // C E G C

uint8_t pcfState = 0xFF;
uint8_t lastButtonBits = 0x0F;
bool    wasDown[4] = {false, false, false, false};

bool pcfPresent() {
  Wire.beginTransmission(PCF8574_ADDR);
  return Wire.endTransmission() == 0;
}

void pcfWrite(uint8_t value) {
  pcfState = value;
  Wire.beginTransmission(PCF8574_ADDR);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t pcfRead() {
  Wire.requestFrom((uint8_t)PCF8574_ADDR, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0xFF;
}

void setLed(int n, bool on) {
  if (on) pcfState |=  (1 << LED_BIT[n]);
  else    pcfState &= ~(1 << LED_BIT[n]);
  pcfWrite(pcfState);
}

void printNibble(uint8_t v) {
  for (int b = 3; b >= 0; b--) Serial.print((v >> b) & 1);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== SIMON SAYS : BUTTON TEST ==="));

  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  if (!pcfPresent()) {
    Serial.print(F("[FAIL] No PCF8574 found at 0x"));
    Serial.println(PCF8574_ADDR, HEX);
    Serial.println(F("       Check SDA/SCL wiring, power, and the A0-A2 address pins."));
    Serial.println(F("       (PCF8574A chips live at 0x38-0x3F instead of 0x20-0x27.)"));
    while (true) delay(1000);
  }

  Serial.print(F("[ OK ] PCF8574 found at 0x"));
  Serial.println(PCF8574_ADDR, HEX);

  // LEDs off, button pins high (input mode)
  pcfWrite(0xF0);

  // Little start-up chase so you can see the LEDs work
  for (int i = 0; i < 4; i++) {
    setLed(i, true);
    tone(BUZZER_PIN, NOTE[i], 80);
    delay(120);
    setLed(i, false);
  }
  noTone(BUZZER_PIN);

  Serial.println(F("Press buttons 1-4 ..."));
  Serial.println(F("-------------------------------"));
}

void loop() {
  uint8_t raw = pcfRead();

  // Print the button pins (P7..P4) whenever they change - 0 means pressed
  uint8_t buttonBits = raw >> 4;
  if (buttonBits != lastButtonBits) {
    Serial.print(F("buttons P7..P4 = "));
    printNibble(buttonBits);
    Serial.println();
    lastButtonBits = buttonBits;
  }

  for (int i = 0; i < 4; i++) {
    bool down = !(raw & (1 << BUTTON_BIT[i]));   // active LOW

    if (down && !wasDown[i]) {
      Serial.print(F("  BUTTON "));
      Serial.print(i + 1);
      Serial.println(F("  pressed"));
      setLed(i, true);
      tone(BUZZER_PIN, NOTE[i]);
    }

    if (!down && wasDown[i]) {
      Serial.print(F("  BUTTON "));
      Serial.print(i + 1);
      Serial.println(F("  released"));
      setLed(i, false);
      noTone(BUZZER_PIN);
    }

    wasDown[i] = down;
  }

  delay(10);
}
