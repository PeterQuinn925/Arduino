// ============================================================
//  74LS193N Synchronous 4-Bit Up/Down Binary Counter Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Control inputs:
//   D2  (MR)   [14]  Master Reset, ACTIVE HIGH (LOW=inactive)
//                     NOTE: opposite polarity from 74LS161's /MR!
//   D3  (CPU)  [5]   Count Up clock, rising edge
//   D4  (CPD)  [4]   Count Down clock, rising edge
//   D5  (/PL)  [11]  Parallel Load, active LOW (HIGH=count mode)
//
//  Parallel data inputs (loaded when /PL=LOW):
//   D6  (A)    [15]  Data bit 0 (LSB) -> QA
//   D7  (B)    [1]   Data bit 1       -> QB
//   D8  (C)    [10]  Data bit 2       -> QC
//   D9  (D)    [9]   Data bit 3 (MSB) -> QD
//
//  Outputs:
//   A0  (QA)   [3]   Output bit 0 (LSB)
//   A1  (QB)   [2]   Output bit 1
//   A2  (QC)   [6]   Output bit 2
//   A3  (QD)   [7]   Output bit 3 (MSB)
//   D10 (/TCU) [12]  Terminal Count Up, active LOW (pulses LOW at max count overflow)
//   D11 (/TCD) [13]  Terminal Count Down, active LOW (pulses LOW at min count underflow)
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  IMPORTANT: When counting up, CPD must be held HIGH.
//             When counting down, CPU must be held HIGH.
//             Both LOW or both toggling gives undefined behavior.
//
//  Mode select:
//   MR    /PL   | Action
//   H     X     | Async Clear (Q=0000, overrides clock and load)
//   L     L     | Async Parallel Load (P -> Q, overrides clock)
//   L     H     | Count mode: CPU rising edge counts up,
//                              CPD rising edge counts down
//                (the other clock must be held HIGH while counting)
//
//  Terminal Count:
//   /TCU goes LOW when count=15 (1111) AND CPU is LOW
//   /TCD goes LOW when count=0  (0000) AND CPD is LOW
// ============================================================

// --- Control pins ---
const int PIN_MR  = 2;    // active HIGH
const int PIN_CPU = 3;    // count up clock
const int PIN_CPD = 4;    // count down clock
const int PIN_PL  = 5;    // active LOW

// --- Parallel data input pins ---
const int PIN_DATA[4] = {6, 7, 8, 9};   // A, B, C, D

// --- Output pins ---
const int PIN_Q[4] = {A0, A1, A2, A3};  // QA..QD
const int PIN_TCU = 10;
const int PIN_TCD = 11;

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Read Q outputs as a 4-bit value
// ---------------------------------------------------------------
uint8_t readCount() {
  delay(10);
  uint8_t val = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(PIN_Q[i])) val |= (1 << i);
  }
  return val;
}

// ---------------------------------------------------------------
// Rising edge pulse on count-up clock.
// CPD must already be HIGH before calling.
// ---------------------------------------------------------------
void pulseCountUp() {
  digitalWrite(PIN_CPU, LOW);
  delay(15);
  digitalWrite(PIN_CPU, HIGH);   // rising edge — increments
  delay(15);
}

// ---------------------------------------------------------------
// Rising edge pulse on count-down clock.
// CPU must already be HIGH before calling.
// ---------------------------------------------------------------
void pulseCountDown() {
  digitalWrite(PIN_CPD, LOW);
  delay(15);
  digitalWrite(PIN_CPD, HIGH);   // rising edge — decrements
  delay(15);
}

// ---------------------------------------------------------------
// Async master reset (active HIGH) — clears Q to 0 immediately
// ---------------------------------------------------------------
void asyncReset() {
  digitalWrite(PIN_MR, HIGH);
  delay(30);
  digitalWrite(PIN_MR, LOW);
  delay(20);
}

// ---------------------------------------------------------------
// Apply parallel data A-D
// ---------------------------------------------------------------
void applyData(uint8_t val) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(PIN_DATA[i], (val >> i) & 1);
  }
}

// ---------------------------------------------------------------
// Async parallel load — /PL LOW loads data immediately, no clock
// ---------------------------------------------------------------
void parallelLoad(uint8_t val) {
  applyData(val);
  delay(10);
  digitalWrite(PIN_PL, LOW);
  delay(30);
  digitalWrite(PIN_PL, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Print 4-bit binary
// ---------------------------------------------------------------
void printBin4(uint8_t val) {
  for (int i = 3; i >= 0; i--) Serial.print((val >> i) & 1);
}

// ---------------------------------------------------------------
// Print result row
// ---------------------------------------------------------------
bool printResult(const char* phase, const char* mode,
                 uint8_t actual, uint8_t expected,
                 int actualTCU, int expectedTCU,
                 int actualTCD, int expectedTCD) {
  bool ok = (actual == expected) &&
           (actualTCU == expectedTCU) &&
           (actualTCD == expectedTCD);
  if (ok) totalPass++; else totalFail++;

  Serial.print(F("  "));
  int plen = strlen(phase);
  Serial.print(phase);
  for (int i = plen; i < 8; i++) Serial.print(' ');
  Serial.print(F("  "));
  int mlen = strlen(mode);
  Serial.print(mode);
  for (int i = mlen; i < 24; i++) Serial.print(' ');
  Serial.print(F("  Q="));
  printBin4(actual);
  Serial.print(F(" ("));
  if (actual < 10) Serial.print(' ');
  Serial.print(actual);
  Serial.print(F(") TCU="));
  Serial.print(actualTCU);
  Serial.print(F(" TCD="));
  Serial.print(actualTCD);
  Serial.print(F("  exp="));
  printBin4(expected);
  Serial.print(F(" ("));
  if (expected < 10) Serial.print(' ');
  Serial.print(expected);
  Serial.print(F(") TCU="));
  Serial.print(expectedTCU);
  Serial.print(F(" TCD="));
  Serial.print(expectedTCD);
  Serial.print(F("  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));
  return ok;
}

void printSep() {
  Serial.println(F("  ----------------------------------------------------------------------------------------"));
}

void printHeader() {
  Serial.println(F("  Phase     Mode                      Q=xxxx (n) TCU/TCD  exp=xxxx (n) TCU/TCD  result"));
  printSep();
}

// ---------------------------------------------------------------
// Phase 1: Async Master Reset (active HIGH)
// ---------------------------------------------------------------
void testAsyncReset() {
  Serial.println(F("\n--- Phase 1: Async Master Reset (MR, active HIGH) ---"));
  Serial.println(F("  MR HIGH clears Q immediately, no clock required"));
  printHeader();

  // Load a known non-zero value
  digitalWrite(PIN_MR, LOW);
  delay(10);
  parallelLoad(0b1010);
  printResult("Setup", "Load 1010 before reset  ", readCount(), 10,
             digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 1);

  // Assert MR — clears immediately
  digitalWrite(PIN_MR, HIGH);
  delay(30);
  printResult("Async", "MR asserted (no clock)  ", readCount(), 0,
             digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 0);
  digitalWrite(PIN_MR, LOW);
  delay(20);
  printResult("Async", "MR released              ", readCount(), 0,
             digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 0);

  // MR overrides clock
  digitalWrite(PIN_MR, HIGH);
  delay(20);
  digitalWrite(PIN_CPD, HIGH);
  pulseCountUp();
  printResult("Async", "MR=H during CLK pulse    ", readCount(), 0,
             digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 0);
  digitalWrite(PIN_MR, LOW);
  delay(20);
}

// ---------------------------------------------------------------
// Phase 2: Async Parallel Load
// /PL LOW loads data immediately, no clock needed, overrides clock
// ---------------------------------------------------------------
void testParallelLoad() {
  Serial.println(F("\n--- Phase 2: Async Parallel Load (/PL) ---"));
  Serial.println(F("  /PL LOW loads A-D to Q immediately, no clock required"));
  printHeader();

  asyncReset();

  uint8_t testVals[] = {0,1,5,10,15,9,6,12};
  const char* labels[] = {
    "Load 0  (0000)          ",
    "Load 1  (0001)          ",
    "Load 5  (0101)          ",
    "Load 10 (1010)          ",
    "Load 15 (1111)          ",
    "Load 9  (1001)          ",
    "Load 6  (0110)          ",
    "Load 12 (1100)          "
  };

  for (int i = 0; i < 8; i++) {
    uint8_t val = testVals[i];
    parallelLoad(val);
    int expTCU = (val == 15) ? 0 : 1;
    int expTCD = (val == 0)  ? 0 : 1;
    printResult("Load", labels[i], readCount(), val,
               digitalRead(PIN_TCU), expTCU,
               digitalRead(PIN_TCD), expTCD);
  }

  // Verify load overrides clock — set CPU/CPD toggling, assert /PL
  digitalWrite(PIN_CPU, HIGH);
  digitalWrite(PIN_CPD, HIGH);
  applyData(7);
  digitalWrite(PIN_PL, LOW);
  delay(10);
  digitalWrite(PIN_CPU, LOW);   // try to clock while /PL asserted
  delay(10);
  digitalWrite(PIN_CPU, HIGH);
  delay(10);
  printResult("Load", "PL overrides CLK (val=7)", readCount(), 7,
             digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 1);
  digitalWrite(PIN_PL, HIGH);
  delay(10);
}

// ---------------------------------------------------------------
// Phase 3: Count Up sequence 0->15->0 (wrap)
// CPD held HIGH throughout
// ---------------------------------------------------------------
void testCountUp() {
  Serial.println(F("\n--- Phase 3: Count Up (0 -> 15 -> 0) ---"));
  Serial.println(F("  CPD held HIGH; CPU pulsed to count up"));
  Serial.println(F("  /TCU goes LOW only when count=15 (next pulse will overflow)"));
  printHeader();

  asyncReset();
  digitalWrite(PIN_PL, HIGH);
  digitalWrite(PIN_CPD, HIGH);   // must be HIGH while counting up
  digitalWrite(PIN_CPU, HIGH);

  for (uint8_t expected = 0; expected <= 16; expected++) {
    uint8_t expCount = expected % 16;
    // /TCU goes LOW when count=15 and CPU is LOW (mid-pulse)
    // We read it after CPU returns HIGH, so it should read HIGH (inactive)
    char modeBuf[26];
    snprintf(modeBuf, sizeof(modeBuf), "CountUp -> %2d            ", expCount);
    printResult("CountUp", modeBuf, readCount(), expCount,
               digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 1);
    if (expected < 16) pulseCountUp();
  }
}

// ---------------------------------------------------------------
// Phase 4: Count Down sequence 15->0->15 (wrap)
// CPU held HIGH throughout
// ---------------------------------------------------------------
void testCountDown() {
  Serial.println(F("\n--- Phase 4: Count Down (15 -> 0 -> 15) ---"));
  Serial.println(F("  CPU held HIGH; CPD pulsed to count down"));
  printHeader();

  asyncReset();
  parallelLoad(15);
  digitalWrite(PIN_CPU, HIGH);   // must be HIGH while counting down
  digitalWrite(PIN_CPD, HIGH);

  for (int expected = 15; expected >= -1; expected--) {
    uint8_t expCount = (expected < 0) ? 15 : expected;
    char modeBuf[26];
    snprintf(modeBuf, sizeof(modeBuf), "CountDn -> %2d            ", expCount);
    printResult("CountDn", modeBuf, readCount(), expCount,
               digitalRead(PIN_TCU), 1, digitalRead(PIN_TCD), 1);
    if (expected > -1) pulseCountDown();
  }
}

// ---------------------------------------------------------------
// Phase 5: Direction reversal — count up then down then up
// ---------------------------------------------------------------
void testDirectionReversal() {
  Serial.println(F("\n--- Phase 5: Direction Reversal ---"));
  Serial.println(F("  Count up a few steps, then down, then up again"));
  printHeader();

  asyncReset();
  digitalWrite(PIN_PL, HIGH);
  digitalWrite(PIN_CPU, HIGH);
  digitalWrite(PIN_CPD, HIGH);

  uint8_t expected = 0;
  printResult("Reverse", "Start at 0               ", readCount(), expected, 1,1,1,1);

  for (int i = 0; i < 4; i++) { pulseCountUp(); expected++; }
  printResult("Reverse", "After 4x up (0->4)       ", readCount(), expected, 1,1,1,1);

  for (int i = 0; i < 2; i++) { pulseCountDown(); expected--; }
  printResult("Reverse", "After 2x down (4->2)     ", readCount(), expected, 1,1,1,1);

  for (int i = 0; i < 3; i++) { pulseCountUp(); expected++; }
  printResult("Reverse", "After 3x up (2->5)       ", readCount(), expected, 1,1,1,1);

  for (int i = 0; i < 5; i++) { pulseCountDown(); expected--; }
  printResult("Reverse", "After 5x down (5->0)     ", readCount(), expected, 1,1,1,1);
}

// ---------------------------------------------------------------
// Phase 6: Terminal Count outputs
// /TCU pulses LOW when CPU goes LOW while count=15
// /TCD pulses LOW when CPD goes LOW while count=0
// ---------------------------------------------------------------
void testTerminalCount() {
  Serial.println(F("\n--- Phase 6: Terminal Count Outputs ---"));
  Serial.println(F("  /TCU pulses LOW when CPU=LOW and count=15"));
  Serial.println(F("  /TCD pulses LOW when CPD=LOW and count=0"));
  printHeader();

  // Set up at count=15, CPD=HIGH, then bring CPU LOW and check /TCU
  asyncReset();
  parallelLoad(15);
  digitalWrite(PIN_CPD, HIGH);
  digitalWrite(PIN_CPU, HIGH);
  delay(10);
  printResult("TC", "At 15, CPU=H /TCU=H      ", readCount(), 15, 1, 1, 1, 1);

  digitalWrite(PIN_CPU, LOW);   // bring CPU low while at 15
  delay(15);
  printResult("TC", "At 15, CPU=L /TCU=L      ", readCount(), 15, 0, 0, 1, 1);
  digitalWrite(PIN_CPU, HIGH);  // this increments to 0 (wraps)
  delay(15);
  printResult("TC", "CPU back HIGH, wraps to 0", readCount(), 0, 1, 1, 1, 1);

  // Set up at count=0, CPU=HIGH, then bring CPD LOW and check /TCD
  asyncReset();
  digitalWrite(PIN_CPU, HIGH);
  digitalWrite(PIN_CPD, HIGH);
  delay(10);
  printResult("TC", "At 0, CPD=H /TCD=H       ", readCount(), 0, 1, 1, 1, 1);

  digitalWrite(PIN_CPD, LOW);   // bring CPD low while at 0
  delay(15);
  printResult("TC", "At 0, CPD=L /TCD=L       ", readCount(), 0, 1, 1, 0, 0);
  digitalWrite(PIN_CPD, HIGH);  // this decrements to 15 (wraps)
  delay(15);
  printResult("TC", "CPD back HIGH, wraps to15", readCount(), 15, 1, 1, 1, 1);
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Safe states before enabling outputs
  digitalWrite(PIN_MR,  LOW);    // inactive (active HIGH chip)
  digitalWrite(PIN_CPU, HIGH);
  digitalWrite(PIN_CPD, HIGH);
  digitalWrite(PIN_PL,  HIGH);   // count mode (not loading)
  for (int i = 0; i < 4; i++) digitalWrite(PIN_DATA[i], LOW);

  pinMode(PIN_MR,  OUTPUT);
  pinMode(PIN_CPU, OUTPUT);
  pinMode(PIN_CPD, OUTPUT);
  pinMode(PIN_PL,  OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_DATA[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_Q[i], INPUT);
  pinMode(PIN_TCU, INPUT);
  pinMode(PIN_TCD, INPUT);

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS193N 4-Bit Up/Down Counter Tester"));
  Serial.println(F("========================================"));

  testAsyncReset();
  testParallelLoad();
  testCountUp();
  testCountDown();
  testDirectionReversal();
  testTerminalCount();

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
