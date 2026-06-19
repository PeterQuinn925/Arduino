// ============================================================
//  74LS112N Dual JK Negative Edge-Triggered Flip-Flop Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Flip-Flop 1:
//   D2  (1J)    [1]   J input
//   D3  (1CLK)  [2]   Clock (triggers on FALLING edge)
//   D4  (1K)    [3]   K input
//   D5  (1PRE)  [4]   Active LOW preset  (hold HIGH when inactive)
//   D6  (1CLR)  [15]  Active LOW clear   (hold HIGH when inactive)
//   A0  (1Q)    [5]   Output Q
//   A1  (1QB)   [6]   Output /Q
//
//  Flip-Flop 2:
//   D7  (2J)    [14]  J input
//   D8  (2CLK)  [13]  Clock (triggers on FALLING edge)
//   D9  (2K)    [12]  K input
//   D10 (2PRE)  [11]  Active LOW preset  (hold HIGH when inactive)
//   D11 (2CLR)  [10]  Active LOW clear   (hold HIGH when inactive)
//   A2  (2Q)    [9]   Output Q
//   A3  (2QB)   [7]   Output /Q
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  NOTE: Clock triggers on the FALLING edge (HIGH->LOW transition).
//        /PRE and /CLR are active LOW — always held HIGH when inactive.
//        Q and /Q are always complementary on a working chip.
// ============================================================

// --- Flip-Flop 1 pins ---
const int FF1_J   = 2;
const int FF1_CLK = 3;
const int FF1_K   = 4;
const int FF1_PRE = 5;
const int FF1_CLR = 6;
const int FF1_Q   = A0;
const int FF1_QB  = A1;

// --- Flip-Flop 2 pins ---
const int FF2_J   = 7;
const int FF2_CLK = 8;
const int FF2_K   = 9;
const int FF2_PRE = 10;
const int FF2_CLR = 11;
const int FF2_Q   = A2;
const int FF2_QB  = A3;

bool ffPass[2];

// ---------------------------------------------------------------
// Diagnostic: print the raw state of all pins for one FF
// ---------------------------------------------------------------
void printPinStates(int ffNum,
                    int pinJ, int pinCLK, int pinK,
                    int pinPRE, int pinCLR,
                    int pinQ, int pinQB) {
  Serial.print(F("  [DIAG FF"));
  Serial.print(ffNum);
  Serial.print(F("] J="));    Serial.print(digitalRead(pinJ));
  Serial.print(F(" CLK="));   Serial.print(digitalRead(pinCLK));
  Serial.print(F(" K="));     Serial.print(digitalRead(pinK));
  Serial.print(F(" /PRE="));  Serial.print(digitalRead(pinPRE));
  Serial.print(F(" /CLR="));  Serial.print(digitalRead(pinCLR));
  Serial.print(F(" Q="));     Serial.print(digitalRead(pinQ));
  Serial.print(F(" /Q="));    Serial.println(digitalRead(pinQB));
}

// ---------------------------------------------------------------
// Pulse clock LOW (falling edge).
// Chip captures J/K on HIGH->LOW transition.
// Prints pin states before and after for diagnostics.
// ---------------------------------------------------------------
void pulseClock(int ffNum,
                int clkPin,
                int pinJ, int pinK,
                int pinPRE, int pinCLR,
                int pinQ, int pinQB) {
  Serial.print(F("  [CLK FF"));
  Serial.print(ffNum);
  Serial.print(F("] Before pulse: J="));
  Serial.print(digitalRead(pinJ));
  Serial.print(F(" K="));
  Serial.print(digitalRead(pinK));
  Serial.print(F(" Q="));
  Serial.print(digitalRead(pinQ));
  Serial.print(F(" /Q="));
  Serial.println(digitalRead(pinQB));

  digitalWrite(clkPin, HIGH);
  delay(50);
  digitalWrite(clkPin, LOW);    // falling edge — J/K captured here
  delay(50);
  digitalWrite(clkPin, HIGH);   // return HIGH ready for next pulse
  delay(50);

  Serial.print(F("  [CLK FF"));
  Serial.print(ffNum);
  Serial.print(F("] After  pulse: J="));
  Serial.print(digitalRead(pinJ));
  Serial.print(F(" K="));
  Serial.print(digitalRead(pinK));
  Serial.print(F(" Q="));
  Serial.print(digitalRead(pinQ));
  Serial.print(F(" /Q="));
  Serial.println(digitalRead(pinQB));
}

// ---------------------------------------------------------------
// Assert /PRE LOW -> Q=1
// ---------------------------------------------------------------
void assertPreset(int ffNum, int prePin, int qPin, int qbPin) {
  Serial.print(F("  [PRE FF"));
  Serial.print(ffNum);
  Serial.print(F("] Asserting /PRE LOW -> pin reads: "));
  digitalWrite(prePin, LOW);
  delay(10);
  Serial.print(F("Q="));
  Serial.print(digitalRead(qPin));
  Serial.print(F(" /Q="));
  Serial.println(digitalRead(qbPin));
  delay(50);
}

void releasePreset(int prePin) {
  digitalWrite(prePin, HIGH);
  delay(30);
}

// ---------------------------------------------------------------
// Assert /CLR LOW -> Q=0
// ---------------------------------------------------------------
void assertClear(int ffNum, int clrPin, int qPin, int qbPin) {
  Serial.print(F("  [CLR FF"));
  Serial.print(ffNum);
  Serial.print(F("] Asserting /CLR LOW -> pin reads: "));
  digitalWrite(clrPin, LOW);
  delay(10);
  Serial.print(F("Q="));
  Serial.print(digitalRead(qPin));
  Serial.print(F(" /Q="));
  Serial.println(digitalRead(qbPin));
  delay(50);
}

void releaseClear(int clrPin) {
  digitalWrite(clrPin, HIGH);
  delay(30);
}

// ---------------------------------------------------------------
// Put flip-flop into known Q=0 state
// ---------------------------------------------------------------
void resetFF(int ffNum,
             int prePin, int clrPin, int clkPin,
             int jPin, int kPin,
             int qPin, int qbPin) {
  releasePreset(prePin);
  assertClear(ffNum, clrPin, qPin, qbPin);
  releaseClear(clrPin);
  digitalWrite(clkPin, HIGH);
  digitalWrite(jPin,   LOW);
  digitalWrite(kPin,   LOW);
  delay(30);
}

// ---------------------------------------------------------------
// Check outputs, print result row, return pass/fail
// ---------------------------------------------------------------
bool checkOutputs(int qPin, int qbPin,
                  int expectedQ,
                  const char* phaseName,
                  const char* modeName) {
  delay(20);
  int actualQ  = digitalRead(qPin);
  int actualQB = digitalRead(qbPin);
  int expectedQB = !expectedQ;

  bool qOk  = (actualQ  == expectedQ);
  bool qbOk = (actualQB == expectedQB);
  bool ok   = qOk && qbOk;

  Serial.print(F("  "));
  int plen = strlen(phaseName);
  Serial.print(phaseName);
  for (int i = plen; i < 10; i++) Serial.print(' ');
  Serial.print(F("  "));
  int mlen = strlen(modeName);
  Serial.print(modeName);
  for (int i = mlen; i < 22; i++) Serial.print(' ');
  Serial.print(F("  Q="));
  Serial.print(actualQ);
  Serial.print(F(" /Q="));
  Serial.print(actualQB);
  Serial.print(F("  exp Q="));
  Serial.print(expectedQ);
  Serial.print(F(" /Q="));
  Serial.print(expectedQB);
  Serial.print(F("  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));

  return ok;
}

// ---------------------------------------------------------------
// Test one JK flip-flop
// ---------------------------------------------------------------
bool testFF(int ffNum,
            int pinJ,   int pinCLK, int pinK,
            int pinPRE, int pinCLR,
            int pinQ,   int pinQB) {

  Serial.print(F("\n--- Flip-Flop "));
  Serial.print(ffNum);
  Serial.println(F(" ---"));
  Serial.println(F("  Phase       Mode                    Q /Q  exp Q /Q  result"));
  Serial.println(F("  ---------------------------------------------------------------"));

  bool allOk = true;

  // Known starting state
  resetFF(ffNum, pinPRE, pinCLR, pinCLK, pinJ, pinK, pinQ, pinQB);
  printPinStates(ffNum, pinJ, pinCLK, pinK, pinPRE, pinCLR, pinQ, pinQB);

  // ---- Phase 1: Async Preset ----
  assertClear(ffNum, pinCLR, pinQ, pinQB);   // ensure Q=0 first
  releaseClear(pinCLR);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Async", "Before preset (Q=0)");
  assertPreset(ffNum, pinPRE, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 1, "Async", "PRE asserted (Q->1)");
  releasePreset(pinPRE);
  allOk &= checkOutputs(pinQ, pinQB, 1, "Async", "PRE released (Q=1) ");

  // ---- Phase 2: Async Clear ----
  allOk &= checkOutputs(pinQ, pinQB, 1, "Async", "Before clear (Q=1) ");
  assertClear(ffNum, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Async", "CLR asserted (Q->0)");
  releaseClear(pinCLR);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Async", "CLR released (Q=0) ");

  printPinStates(ffNum, pinJ, pinCLK, pinK, pinPRE, pinCLR, pinQ, pinQB);

  // ---- Phase 3: Clocked SET (J=1, K=0) ----
  assertClear(ffNum, pinCLR, pinQ, pinQB);
  releaseClear(pinCLR);
  digitalWrite(pinJ, HIGH);
  digitalWrite(pinK, LOW);
  delay(50);
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 1, "Clocked", "J=1 K=0 SET (Q->1)");

  // ---- Phase 4: Clocked RESET (J=0, K=1) ----
  digitalWrite(pinJ, LOW);
  digitalWrite(pinK, HIGH);
  delay(50);
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Clocked", "J=0 K=1 RST (Q->0)");

  // ---- Phase 5: Clocked HOLD ----
  digitalWrite(pinJ, LOW);
  digitalWrite(pinK, LOW);
  delay(50);
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Clocked", "J=0 K=0 HOLD (Q=0)");
  // Set to 1 then hold
  digitalWrite(pinJ, HIGH);
  digitalWrite(pinK, LOW);
  delay(50);
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  digitalWrite(pinJ, LOW);
  digitalWrite(pinK, LOW);
  delay(50);
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 1, "Clocked", "J=0 K=0 HOLD (Q=1)");

  // ---- Phase 6: Toggle ----
  digitalWrite(pinJ, HIGH);
  digitalWrite(pinK, HIGH);
  delay(50);
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Clocked", "J=1 K=1 TOG (1->0)");
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 1, "Clocked", "J=1 K=1 TOG (0->1)");
  pulseClock(ffNum, pinCLK, pinJ, pinK, pinPRE, pinCLR, pinQ, pinQB);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Clocked", "J=1 K=1 TOG (1->0)");

  // ---- Phase 7: Hold without clock edge ----
  digitalWrite(pinJ, HIGH);
  digitalWrite(pinK, LOW);
  delay(100);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Hold", "No CLK edge (Q=0) ");
  digitalWrite(pinJ, LOW);
  digitalWrite(pinK, HIGH);
  delay(100);
  allOk &= checkOutputs(pinQ, pinQB, 0, "Hold", "No CLK edge (Q=0) ");

  printPinStates(ffNum, pinJ, pinCLK, pinK, pinPRE, pinCLR, pinQ, pinQB);

  resetFF(ffNum, pinPRE, pinCLR, pinCLK, pinJ, pinK, pinQ, pinQB);
  return allOk;
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // FF1 — safe states before enabling outputs
  digitalWrite(FF1_PRE, HIGH);
  digitalWrite(FF1_CLR, HIGH);
  digitalWrite(FF1_CLK, HIGH);
  digitalWrite(FF1_J,   LOW);
  digitalWrite(FF1_K,   LOW);
  pinMode(FF1_PRE, OUTPUT);
  pinMode(FF1_CLR, OUTPUT);
  pinMode(FF1_CLK, OUTPUT);
  pinMode(FF1_J,   OUTPUT);
  pinMode(FF1_K,   OUTPUT);
  pinMode(FF1_Q,   INPUT);
  pinMode(FF1_QB,  INPUT);

  // FF2
  digitalWrite(FF2_PRE, HIGH);
  digitalWrite(FF2_CLR, HIGH);
  digitalWrite(FF2_CLK, HIGH);
  digitalWrite(FF2_J,   LOW);
  digitalWrite(FF2_K,   LOW);
  pinMode(FF2_PRE, OUTPUT);
  pinMode(FF2_CLR, OUTPUT);
  pinMode(FF2_CLK, OUTPUT);
  pinMode(FF2_J,   OUTPUT);
  pinMode(FF2_K,   OUTPUT);
  pinMode(FF2_Q,   INPUT);
  pinMode(FF2_QB,  INPUT);

  delay(200);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS112N Dual JK Flip-Flop Tester"));
  Serial.println(F("  Triggers on FALLING clock edge"));
  Serial.println(F("========================================"));

  ffPass[0] = testFF(1,
                     FF1_J, FF1_CLK, FF1_K, FF1_PRE, FF1_CLR,
                     FF1_Q, FF1_QB);

  ffPass[1] = testFF(2,
                     FF2_J, FF2_CLK, FF2_K, FF2_PRE, FF2_CLR,
                     FF2_Q, FF2_QB);

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
