// ============================================================
//  74LS74N Dual D-Type Edge-Triggered Flip-Flop Tester
//  Arduino Nano
//
//  Wiring: [chip pin] (signal name) Arduino pin
//
//  Flip-Flop 1:
//   D2  (1CLR) [1]   Active LOW clear
//   D3  (1D)   [2]   Data input
//   D4  (1CLK) [3]   Clock (rising edge triggered)
//   D5  (1PRE) [4]   Active LOW preset
//   A0  (1Q)   [5]   Output
//   A1  (1QB)  [6]   Inverted output
//
//  Flip-Flop 2:
//   D6  (2CLR) [13]  Active LOW clear
//   D7  (2D)   [12]  Data input
//   D8  (2CLK) [11]  Clock (rising edge triggered)
//   D9  (2PRE) [10]  Active LOW preset
//   A2  (2Q)   [9]   Output
//   A3  (2QB)  [8]   Inverted output
//
//   GND [7]  -> GND
//   VCC [14] -> 5V
//
//  NOTE: /PRE and /CLR are active LOW — held HIGH when inactive.
//        Q and /Q should always be complementary.
//        If they match each other the chip or connection is faulty.
// ============================================================

// --- Flip-flop 1 pin definitions ---
const int FF1_CLR  = 2;
const int FF1_D    = 3;
const int FF1_CLK  = 4;
const int FF1_PRE  = 5;
const int FF1_Q    = A0;
const int FF1_QB   = A1;

// --- Flip-flop 2 pin definitions ---
const int FF2_CLR  = 6;
const int FF2_D    = 7;
const int FF2_CLK  = 8;
const int FF2_PRE  = 9;
const int FF2_Q    = A2;
const int FF2_QB   = A3;

bool ffPass[2];

// ---------------------------------------------------------------
// Pulse the clock — D must already be stable before calling this.
// Wide pulses used to avoid any timing marginal conditions.
// ---------------------------------------------------------------
void pulseClock(int clkPin) {
  digitalWrite(clkPin, LOW);
  delay(20);                  // hold LOW
  digitalWrite(clkPin, HIGH); // rising edge — D captured here
  delay(20);                  // hold HIGH
  digitalWrite(clkPin, LOW);  // return LOW
  delay(20);                  // let output settle before caller reads
}

// ---------------------------------------------------------------
// Assert /CLR — Q goes LOW asynchronously
// ---------------------------------------------------------------
void assertClear(int clrPin) {
  digitalWrite(clrPin, LOW);
  delay(50);                  // wait for async output to settle
}

void releaseClear(int clrPin) {
  digitalWrite(clrPin, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Assert /PRE — Q goes HIGH asynchronously
// ---------------------------------------------------------------
void assertPreset(int prePin) {
  digitalWrite(prePin, LOW);
  delay(50);
}

void releasePreset(int prePin) {
  digitalWrite(prePin, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Put flip-flop into a known state (Q=LOW) before testing
// ---------------------------------------------------------------
void resetFF(int clrPin, int prePin, int clkPin) {
  releasePreset(prePin);
  assertClear(clrPin);
  releaseClear(clrPin);
  digitalWrite(clkPin, LOW);
  delay(20);
}

// ---------------------------------------------------------------
// Check Q and /Q, print result, return true if both correct
// ---------------------------------------------------------------
bool checkOutputs(int qPin, int qbPin, int expectedQ, const char* testName) {
  delay(10);                   // extra settle time before reading
  int actualQ  = digitalRead(qPin);
  int actualQB = digitalRead(qbPin);
  int expectedQB = !expectedQ;

  bool qOk  = (actualQ  == expectedQ);
  bool qbOk = (actualQB == expectedQB);
  bool ok   = qOk && qbOk;

  Serial.print(F("  "));
  Serial.print(testName);
  Serial.print(F(": Q="));
  Serial.print(actualQ);
  Serial.print(F(" /Q="));
  Serial.print(actualQB);
  Serial.print(F("  expect Q="));
  Serial.print(expectedQ);
  Serial.print(F(" /Q="));
  Serial.print(expectedQB);
  Serial.println(ok ? F("  PASS") : F("  FAIL <<<"));

  return ok;
}

// ---------------------------------------------------------------
// Test one flip-flop through all phases
// ---------------------------------------------------------------
bool testFF(int ffNum,
            int pinCLR, int pinD, int pinCLK, int pinPRE,
            int pinQ,   int pinQB) {

  Serial.print(F("\n--- Flip-Flop "));
  Serial.print(ffNum);
  Serial.println(F(" ---"));

  bool allOk = true;

  // Put into known state before any test
  resetFF(pinCLR, pinPRE, pinCLK);

  // ---- Phase 1: Async Clear (/CLR LOW forces Q=0) ----
  Serial.println(F(" Phase 1: Async Clear"));
  // First preset so we know Q is HIGH coming in, making the
  // clear result unambiguous
  assertPreset(pinPRE);
  releasePreset(pinPRE);
  allOk &= checkOutputs(pinQ, pinQB, 1, "Before CLR (preset)");
  assertClear(pinCLR);
  allOk &= checkOutputs(pinQ, pinQB, 0, "CLR asserted     ");
  releaseClear(pinCLR);
  allOk &= checkOutputs(pinQ, pinQB, 0, "CLR released     ");

  // ---- Phase 2: Async Preset (/PRE LOW forces Q=1) ----
  Serial.println(F(" Phase 2: Async Preset"));
  // Clear first so Q is LOW coming in
  assertClear(pinCLR);
  releaseClear(pinCLR);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Before PRE (clear)");
  assertPreset(pinPRE);
  allOk &= checkOutputs(pinQ, pinQB, 1, "PRE asserted     ");
  releasePreset(pinPRE);
  allOk &= checkOutputs(pinQ, pinQB, 1, "PRE released     ");

  // ---- Phase 3: Clock in D=1 ----
  Serial.println(F(" Phase 3: Clock D=1"));
  // Clear to known Q=0 state first
  assertClear(pinCLR);
  releaseClear(pinCLR);
  digitalWrite(pinD, HIGH);
  delay(20);                    // D stable before clock edge
  pulseClock(pinCLK);
  allOk &= checkOutputs(pinQ, pinQB, 1, "After CLK D=1    ");

  // ---- Phase 4: Clock in D=0 ----
  Serial.println(F(" Phase 4: Clock D=0"));
  digitalWrite(pinD, LOW);
  delay(20);
  pulseClock(pinCLK);
  allOk &= checkOutputs(pinQ, pinQB, 0, "After CLK D=0    ");

  // ---- Phase 5: Hold — Q must not change without a clock ----
  Serial.println(F(" Phase 5: Hold (no clock)"));
  // Q is currently 0; set D=1 but don't clock — Q must stay 0
  digitalWrite(pinD, HIGH);
  delay(50);
  allOk &= checkOutputs(pinQ, pinQB, 0, "D=1 no CLK (hold)");
  // Set D=0 but don't clock — Q must still stay 0
  digitalWrite(pinD, LOW);
  delay(50);
  allOk &= checkOutputs(pinQ, pinQB, 0, "D=0 no CLK (hold)");

  // ---- Phase 6: Toggle D=1 then D=0 via clock ----
  Serial.println(F(" Phase 6: Toggle via clock"));
  digitalWrite(pinD, HIGH);
  delay(20);
  pulseClock(pinCLK);
  allOk &= checkOutputs(pinQ, pinQB, 1, "CLK D=1 toggle   ");
  digitalWrite(pinD, LOW);
  delay(20);
  pulseClock(pinCLK);
  allOk &= checkOutputs(pinQ, pinQB, 0, "CLK D=0 toggle   ");
  digitalWrite(pinD, HIGH);
  delay(20);
  pulseClock(pinCLK);
  allOk &= checkOutputs(pinQ, pinQB, 1, "CLK D=1 toggle   ");

  // Leave in safe state
  resetFF(pinCLR, pinPRE, pinCLK);

  return allOk;
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // FF1 pins — set safe states before pinMode to avoid glitches
  digitalWrite(FF1_CLR, HIGH);  // inactive
  digitalWrite(FF1_PRE, HIGH);  // inactive
  digitalWrite(FF1_D,   LOW);
  digitalWrite(FF1_CLK, LOW);
  pinMode(FF1_CLR, OUTPUT);
  pinMode(FF1_D,   OUTPUT);
  pinMode(FF1_CLK, OUTPUT);
  pinMode(FF1_PRE, OUTPUT);
  pinMode(FF1_Q,   INPUT);
  pinMode(FF1_QB,  INPUT);

  // FF2 pins
  digitalWrite(FF2_CLR, HIGH);
  digitalWrite(FF2_PRE, HIGH);
  digitalWrite(FF2_D,   LOW);
  digitalWrite(FF2_CLK, LOW);
  pinMode(FF2_CLR, OUTPUT);
  pinMode(FF2_D,   OUTPUT);
  pinMode(FF2_CLK, OUTPUT);
  pinMode(FF2_PRE, OUTPUT);
  pinMode(FF2_Q,   INPUT);
  pinMode(FF2_QB,  INPUT);

  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS74N Dual D Flip-Flop Tester"));
  Serial.println(F("========================================"));

  ffPass[0] = testFF(1, FF1_CLR, FF1_D, FF1_CLK, FF1_PRE, FF1_Q, FF1_QB);
  ffPass[1] = testFF(2, FF2_CLR, FF2_D, FF2_CLK, FF2_PRE, FF2_Q, FF2_QB);

  Serial.println(F("\n========== SUMMARY =========="));
  bool chipOk = true;
  for (int f = 0; f < 2; f++) {
    Serial.print(F("Flip-Flop "));
    Serial.print(f + 1);
    Serial.print(F(": "));
    if (ffPass[f]) {
      Serial.println(F("PASS"));
    } else {
      Serial.println(F("FAIL"));
      chipOk = false;
    }
  }
  Serial.println(F("-----------------------------"));
  Serial.println(chipOk ? F("Chip: GOOD") : F("Chip: FAULTY"));
  Serial.println(F("============================="));
  Serial.println(F("\nPress reset to test again."));
}

// ---------------------------------------------------------------
// loop
// ---------------------------------------------------------------
void loop() {}
