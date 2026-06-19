// ============================================================
//  74LS175N Quad D-Type Flip-Flop Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  CAUTION: The 74LS174N (HEX, single-rail outputs, 6 flip-flops)
//  is on the SAME datasheet and has a DIFFERENT pinout.
//  This sketch is ONLY for the 74LS175N (QUAD, dual-rail Q/Q-bar).
//
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Control inputs (shared by all 4 flip-flops):
//   D2  (MR)  [1]   Master Reset, active LOW (HIGH=inactive)
//   D3  (CP)  [9]   Clock, rising edge triggered (shared)
//
//  Flip-Flop 0:
//   D4  (D0)  [4]   Data input
//   A0  (Q0)  [2]   Output Q
//   A1  (/Q0) [3]   Output /Q
//
//  Flip-Flop 1:
//   D5  (D1)  [5]   Data input
//   A2  (Q1)  [7]   Output Q
//   A3  (/Q1) [6]   Output /Q
//
//  Flip-Flop 2:
//   D6  (D2)  [12]  Data input
//   D7  (Q2)  [10]  Output Q
//   D8  (/Q2) [11]  Output /Q
//
//  Flip-Flop 3:
//   D9  (D3)  [13]  Data input
//   D10 (Q3)  [15]  Output Q
//   D11 (/Q3) [14]  Output /Q
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  Truth table (per flip-flop, MR=HIGH):
//   D  | Q(next)  /Q(next)
//   L  | L        H
//   H  | H        L
//
//  MR=LOW forces ALL Q=LOW, /Q=HIGH immediately (async, no clock).
//
//  NOTE: All 4 flip-flops share the same CP and MR lines.
//        A fault on either will affect all 4 simultaneously.
// ============================================================

// --- Shared control pins ---
const int PIN_MR = 2;
const int PIN_CP = 3;

// --- Flip-Flop 0 ---
const int FF0_D  = 4;
const int FF0_Q  = A0;
const int FF0_QB = A1;

// --- Flip-Flop 1 ---
const int FF1_D  = 5;
const int FF1_Q  = A2;
const int FF1_QB = A3;

// --- Flip-Flop 2 ---
const int FF2_D  = 6;
const int FF2_Q  = 7;
const int FF2_QB = 8;

// --- Flip-Flop 3 ---
const int FF3_D  = 9;
const int FF3_Q  = 10;
const int FF3_QB = 11;

const int FF_D[4]  = {FF0_D,  FF1_D,  FF2_D,  FF3_D};
const int FF_Q[4]  = {FF0_Q,  FF1_Q,  FF2_Q,  FF3_Q};
const int FF_QB[4] = {FF0_QB, FF1_QB, FF2_QB, FF3_QB};

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Rising edge clock pulse, shared by all 4 flip-flops
// ---------------------------------------------------------------
void pulseClock() {
  digitalWrite(PIN_CP, LOW);
  delay(15);
  digitalWrite(PIN_CP, HIGH);   // rising edge — all D inputs captured
  delay(15);
  digitalWrite(PIN_CP, LOW);
  delay(15);
}

// ---------------------------------------------------------------
// Async master reset — clears all 4 flip-flops immediately
// ---------------------------------------------------------------
void asyncReset() {
  digitalWrite(PIN_MR, LOW);
  delay(30);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Apply a 4-bit value to D0-D3
// ---------------------------------------------------------------
void applyData(uint8_t val) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(FF_D[i], (val >> i) & 1);
  }
}

// ---------------------------------------------------------------
// Read all 4 Q outputs as a 4-bit value
// ---------------------------------------------------------------
uint8_t readQ() {
  delay(10);
  uint8_t val = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(FF_Q[i])) val |= (1 << i);
  }
  return val;
}

// ---------------------------------------------------------------
// Read all 4 /Q outputs as a 4-bit value
// ---------------------------------------------------------------
uint8_t readQB() {
  delay(10);
  uint8_t val = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(FF_QB[i])) val |= (1 << i);
  }
  return val;
}

// ---------------------------------------------------------------
// Print 4-bit binary
// ---------------------------------------------------------------
void printBin4(uint8_t val) {
  for (int i = 3; i >= 0; i--) Serial.print((val >> i) & 1);
}

// ---------------------------------------------------------------
// Print result row: checks Q matches expected AND /Q is complement
// ---------------------------------------------------------------
bool printResult(const char* phase, const char* mode, uint8_t expectedQ) {
  uint8_t actualQ  = readQ();
  uint8_t actualQB = readQB();
  uint8_t expectedQB = (~expectedQ) & 0x0F;

  bool ok = (actualQ == expectedQ) && (actualQB == expectedQB);
  if (ok) totalPass++; else totalFail++;

  Serial.print(F("  "));
  int plen = strlen(phase);
  Serial.print(phase);
  for (int i = plen; i < 8; i++) Serial.print(' ');
  Serial.print(F("  "));
  int mlen = strlen(mode);
  Serial.print(mode);
  for (int i = mlen; i < 22; i++) Serial.print(' ');
  Serial.print(F("  Q="));
  printBin4(actualQ);
  Serial.print(F(" /Q="));
  printBin4(actualQB);
  Serial.print(F("  exp Q="));
  printBin4(expectedQ);
  Serial.print(F(" /Q="));
  printBin4(expectedQB);
  Serial.print(F("  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));

  return ok;
}

void printSep() {
  Serial.println(F("  -------------------------------------------------------------------------"));
}

void printHeader() {
  Serial.println(F("  Phase     Mode                    Q=xxxx /Q=xxxx  exp Q=xxxx /Q=xxxx  result"));
  printSep();
}

// ---------------------------------------------------------------
// Phase 1: Async Master Reset
// MR LOW must clear all 4 flip-flops immediately, no clock needed
// ---------------------------------------------------------------
void testAsyncReset() {
  Serial.println(F("\n--- Phase 1: Async Master Reset (MR) ---"));
  Serial.println(F("  MR LOW forces all Q=0, /Q=1 immediately, no clock required"));
  printHeader();

  // Load all 1s first
  applyData(0b1111);
  pulseClock();
  printResult("Setup", "Load 1111 before MR  ", 0b1111);

  // Assert MR — should clear immediately, no clock
  digitalWrite(PIN_MR, LOW);
  delay(30);
  printResult("Async", "MR asserted (no clock)", 0b0000);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
  printResult("Async", "MR released           ", 0b0000);

  // MR must override the clock — load 1111, hold MR LOW, pulse clock
  applyData(0b1111);
  digitalWrite(PIN_MR, LOW);
  delay(20);
  pulseClock();
  printResult("Async", "MR=L during CLK pulse ", 0b0000);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Phase 2: Clocked Data Load
// Each value on D0-D3 loaded to Q on rising clock edge
// ---------------------------------------------------------------
void testClockedLoad() {
  Serial.println(F("\n--- Phase 2: Clocked Data Load ---"));
  Serial.println(F("  Data on D0-D3 transferred to Q on rising clock edge"));
  printHeader();

  asyncReset();

  uint8_t testVals[] = {
    0b0000, 0b1111, 0b0001, 0b0010, 0b0100, 0b1000,
    0b1010, 0b0101, 0b1100, 0b0011, 0b1001, 0b0110
  };
  const char* labels[] = {
    "Load 0000            ",
    "Load 1111            ",
    "Load 0001            ",
    "Load 0010            ",
    "Load 0100            ",
    "Load 1000            ",
    "Load 1010            ",
    "Load 0101            ",
    "Load 1100            ",
    "Load 0011            ",
    "Load 1001            ",
    "Load 0110            "
  };

  for (int i = 0; i < 12; i++) {
    applyData(testVals[i]);
    delay(10);
    pulseClock();
    printResult("Clocked", labels[i], testVals[i]);
  }
}

// ---------------------------------------------------------------
// Phase 3: Hold — D changes without a clock pulse must not affect Q
// ---------------------------------------------------------------
void testHold() {
  Serial.println(F("\n--- Phase 3: Hold (no clock edge) ---"));
  Serial.println(F("  D changes without a clock pulse must not change Q"));
  printHeader();

  asyncReset();
  applyData(0b0101);
  pulseClock();
  printResult("Hold", "Load 0101 baseline   ", 0b0101);

  // Change D without clocking — Q must not change
  applyData(0b1010);
  delay(50);
  printResult("Hold", "D=1010 no CLK (hold) ", 0b0101);

  applyData(0b0000);
  delay(50);
  printResult("Hold", "D=0000 no CLK (hold) ", 0b0101);

  applyData(0b1111);
  delay(50);
  printResult("Hold", "D=1111 no CLK (hold) ", 0b0101);

  // Now actually clock — should pick up the last D value (1111)
  pulseClock();
  printResult("Hold", "CLK pulse (Q->1111)  ", 0b1111);
}

// ---------------------------------------------------------------
// Phase 4: Independence — each flip-flop responds only to its own D
// ---------------------------------------------------------------
void testIndependence() {
  Serial.println(F("\n--- Phase 4: Flip-Flop Independence ---"));
  Serial.println(F("  Walking 1 through D0-D3; only matching FF should show Q=1"));
  printHeader();

  asyncReset();

  for (int bit = 0; bit < 4; bit++) {
    uint8_t val = (1 << bit);
    applyData(val);
    delay(10);
    pulseClock();
    char modeBuf[24];
    snprintf(modeBuf, sizeof(modeBuf), "Walking 1 at D%d        ", bit);
    printResult("Indep", modeBuf, val);
  }

  // Walking 0 (all 1 except one bit)
  for (int bit = 0; bit < 4; bit++) {
    uint8_t val = (~(1 << bit)) & 0x0F;
    applyData(val);
    delay(10);
    pulseClock();
    char modeBuf[24];
    snprintf(modeBuf, sizeof(modeBuf), "Walking 0 at D%d        ", bit);
    printResult("Indep", modeBuf, val);
  }
}

// ---------------------------------------------------------------
// Phase 5: Q and /Q complementary check across full count 0-15
// ---------------------------------------------------------------
void testFullSweep() {
  Serial.println(F("\n--- Phase 5: Full 4-Bit Sweep ---"));
  Serial.println(F("  All 16 combinations loaded; Q and /Q checked each time"));
  printHeader();

  asyncReset();

  for (uint8_t val = 0; val < 16; val++) {
    applyData(val);
    delay(10);
    pulseClock();
    char modeBuf[24];
    snprintf(modeBuf, sizeof(modeBuf), "Load %d                 ", val);
    printResult("Sweep", modeBuf, val);
  }
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Safe states before enabling outputs
  digitalWrite(PIN_MR, HIGH);   // inactive
  digitalWrite(PIN_CP, LOW);
  for (int i = 0; i < 4; i++) digitalWrite(FF_D[i], LOW);

  pinMode(PIN_MR, OUTPUT);
  pinMode(PIN_CP, OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(FF_D[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(FF_Q[i], INPUT);
  for (int i = 0; i < 4; i++) pinMode(FF_QB[i], INPUT);

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS175N Quad D Flip-Flop Tester"));
  Serial.println(F("========================================"));

  testAsyncReset();
  testClockedLoad();
  testHold();
  testIndependence();
  testFullSweep();

  Serial.println(F("\n========== SUMMARY =========="));
  Serial.print(F("  Passed: ")); Serial.println(totalPass);
  Serial.print(F("  Failed: ")); Serial.println(totalFail);
  Serial.println(F("  ----------------------------"));
  Serial.println(totalFail == 0 ? F("  Chip: GOOD") : F("  Chip: FAULTY"));
  Serial.println(F("=============================="));
  Serial.println(F("\nPress reset to test again."));
}

// ---------------------------------------------------------------
// loop
// ---------------------------------------------------------------
void loop() {}
