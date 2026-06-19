// ============================================================
// DM74LS293N / 74LS293 4-Bit Binary / Decade Counter Tester
// Arduino Nano
//
// 14-pin DIP package (CORRECT NATIONAL/TI PINOUT)
//
// Wiring: Arduino pin (signal name) [chip pin]
//
// ------------------------------------------------------------
// CLOCK INPUTS
// D2  (CP0)  [10]   Clock A (LSB section)
// D3  (CP1)  [11]   Clock B (MSB section)
//
// ------------------------------------------------------------
// RESET INPUTS (active HIGH)
// D4  (MR1)  [12]
// D5  (MR2)  [13]
//
// ------------------------------------------------------------
// OUTPUTS
// A0  (QA)   [9]    Bit 0 (LSB)
// A1  (QB)   [5]    Bit 1
// A2  (QC)   [4]    Bit 2
// A3  (QD)   [8]    Bit 3 (MSB)
//
// ------------------------------------------------------------
// POWER
// VCC [14] -> +5V
// GND [7]  -> GND
//
// ------------------------------------------------------------
// MOD-16 CONFIGURATION (IMPORTANT)
//
// Connect:
//   QA [9] -> CP1 [11]
//
// This cascades the two 2-bit sections into a 4-bit counter.
//
// ------------------------------------------------------------
// Expected behavior:
//   0000 → 0001 → 0010 → ... → 1111 → 0000
// ============================================================


// ---------------------- PIN SETUP ----------------------

const int PIN_CP0 = 2;
const int PIN_CP1 = 3;

const int PIN_MR1 = 4;
const int PIN_MR2 = 5;

const int Q_PINS[4] = {A0, A1, A2, A3};

int totalPass = 0;
int totalFail = 0;


// ---------------------- CLOCK PULSE ----------------------

void pulseClock(int pin)
{
  digitalWrite(pin, HIGH);
  delayMicroseconds(20);
  digitalWrite(pin, LOW);
  delayMicroseconds(20);
}


// ---------------------- READ OUTPUTS ----------------------

uint8_t readCounter()
{
  uint8_t value = 0;

  for (int i = 0; i < 4; i++)
  {
    if (digitalRead(Q_PINS[i]))
      value |= (1 << i);
  }

  return value;
}

void pulse293(uint8_t count)
{
  // CP0 always toggles every step
  digitalWrite(PIN_CP0, HIGH);
  delayMicroseconds(20);
  digitalWrite(PIN_CP0, LOW);

  // CP1 toggles only every 2 pulses (simulates QA feedback internally)
  if (count % 2 == 1)
  {
    digitalWrite(PIN_CP1, HIGH);
    delayMicroseconds(20);
    digitalWrite(PIN_CP1, LOW);
  }

  delayMicroseconds(50);
}
// ---------------------- PRINT BINARY ----------------------

void print4bit(uint8_t v)
{
  for (int i = 3; i >= 0; i--)
    Serial.print((v >> i) & 1);
}


// ---------------------- RESET COUNTER ----------------------

void resetCounter()
{
  digitalWrite(PIN_MR1, HIGH);
  digitalWrite(PIN_MR2, HIGH);

  delay(1);

  digitalWrite(PIN_MR1, LOW);
  digitalWrite(PIN_MR2, LOW);

  delay(1);
}


// ---------------------- CHECK VALUE ----------------------

bool check(uint8_t expected)
{
  uint8_t actual = readCounter();

  bool pass = (actual == expected);

  if (pass) totalPass++;
  else totalFail++;

  Serial.print(F("Expected="));
  print4bit(expected);

  Serial.print(F(" ("));
  Serial.print(expected);
  Serial.print(F(")  Actual="));

  print4bit(actual);

  Serial.print(F(" ("));
  Serial.print(actual);
  Serial.print(F(")  "));

  Serial.println(pass ? F("PASS") : F("FAIL <<<"));

  return pass;
}


// ---------------------- RESET TEST ----------------------

void resetTest()
{
  Serial.println(F("\n=== RESET TEST ==="));

  resetCounter();
  check(0);
}


// ---------------------- BASIC COUNT TEST (0–15) ----------------------

void countTest()
{
  Serial.println(F("\n=== MOD-16 COUNT TEST ==="));

  resetCounter();
  check(0);

for (uint8_t i = 0; i < 16; i++)
{
  pulse293(i);
  delay(2);
  check(i);
}
}


// ---------------------- ROLLOVER TEST ----------------------

void rolloverTest()
{
  Serial.println(F("\n=== ROLLOVER TEST (32 pulses) ==="));

  resetCounter();

  for (uint8_t i = 0; i < 32; i++)
  {
    pulseClock(PIN_CP0);
    delay(2);

    check(i & 0x0F);
  }
}


// ---------------------- SETUP ----------------------

void setup()
{
  Serial.begin(9600);
  while (!Serial) {}

  pinMode(PIN_CP0, OUTPUT);
  pinMode(PIN_CP1, OUTPUT);

  pinMode(PIN_MR1, OUTPUT);
  pinMode(PIN_MR2, OUTPUT);

  digitalWrite(PIN_CP0, LOW);
  digitalWrite(PIN_CP1, LOW);
  digitalWrite(PIN_MR1, LOW);
  digitalWrite(PIN_MR2, LOW);

  for (int i = 0; i < 4; i++)
    pinMode(Q_PINS[i], INPUT);

  delay(100);

  Serial.println(F("\n======================================"));
  Serial.println(F("   DM74LS293N COUNTER TESTER"));
  Serial.println(F("   Correct 14-pin DIP Mapping"));
  Serial.println(F("======================================"));

  Serial.println(F("QA->CP1 jumper required for MOD-16"));
  Serial.println(F("======================================"));

  resetTest();
  countTest();
  rolloverTest();

  Serial.println(F("\n============== SUMMARY =============="));

  Serial.print(F("Passed: "));
  Serial.println(totalPass);

  Serial.print(F("Failed: "));
  Serial.println(totalFail);

  if (totalFail == 0)
    Serial.println(F("STATUS: GOOD CHIP"));
  else
    Serial.println(F("STATUS: FAULTY OR WIRING ISSUE"));

  Serial.println(F("====================================="));
  Serial.println(F("Press RESET to rerun."));
}


// ---------------------- LOOP ----------------------

void loop()
{
}