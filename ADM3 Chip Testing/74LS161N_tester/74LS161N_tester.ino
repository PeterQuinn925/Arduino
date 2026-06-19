// ============================================================
//  74LS161N Synchronous 4-Bit Binary Counter Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Control inputs:
//   D2  (/MR)  [1]   Async Master Reset, active LOW (HIGH=inactive)
//   D3  (CP)   [2]   Clock, rising edge triggered
//   D4  (CEP)  [7]   Count Enable Parallel, active HIGH
//   D5  (CET)  [10]  Count Enable Trickle, active HIGH
//   D6  (/PE)  [9]   Parallel Load Enable, active LOW (HIGH=count mode)
//
//  Parallel data inputs (for preset/load):
//   D7  (P0)   [3]   Data bit 0 (LSB)
//   D8  (P1)   [4]   Data bit 1
//   D9  (P2)   [5]   Data bit 2
//   D10 (P3)   [6]   Data bit 3 (MSB)
//
//  Outputs:
//   A0  (Q0)   [14]  Output bit 0 (LSB)
//   A1  (Q1)   [13]  Output bit 1
//   A2  (Q2)   [12]  Output bit 2
//   A3  (Q3)   [11]  Output bit 3 (MSB)
//   D11 (TC)   [15]  Terminal Count (HIGH when count=15 and CET=HIGH)
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  Mode select table:
//   /MR  /PE  CET  CEP  | Action on rising clock edge
//   L    X    X    X    | RESET async (Q=0000 immediately, no clock needed)
//   H    L    X    X    | LOAD (P0-P3 -> Q0-Q3)
//   H    H    H    H    | COUNT (increment)
//   H    H    L    X    | HOLD (no change)
//   H    H    X    L    | HOLD (no change)
//
//  Terminal Count (TC):
//   HIGH only when CET=HIGH and count=15 (1111)
// ============================================================

// --- Control pins ---
const int PIN_MR  = 2;
const int PIN_CP  = 3;
const int PIN_CEP = 4;
const int PIN_CET = 5;
const int PIN_PE  = 6;

// --- Parallel data input pins ---
const int PIN_P[4] = {7, 8, 9, 10};

// --- Output pins ---
const int PIN_Q[4] = {A0, A1, A2, A3};
const int PIN_TC  = 11;

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Read Q0-Q3 as a 4-bit value
// ---------------------------------------------------------------
uint8_t readCount() {
  delay(10);
  uint8_t val = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(PIN_Q[i])) val |= (1 << i);
  }
  return val;
}

int readTC() {
  return digitalRead(PIN_TC);
}

// ---------------------------------------------------------------
// Rising edge clock pulse with generous timing
// All control inputs must be stable before calling
// ---------------------------------------------------------------
void pulseClock() {
  digitalWrite(PIN_CP, LOW);
  delay(20);
  digitalWrite(PIN_CP, HIGH);   // rising edge
  delay(20);
  digitalWrite(PIN_CP, LOW);
  delay(20);
}

// ---------------------------------------------------------------
// Async reset — /MR LOW immediately clears outputs
// ---------------------------------------------------------------
void asyncReset() {
  // Disable count enables first to avoid any race
  digitalWrite(PIN_CEP, LOW);
  digitalWrite(PIN_CET, LOW);
  digitalWrite(PIN_PE,  HIGH);
  delay(10);
  digitalWrite(PIN_MR, LOW);
  delay(30);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
  // Re-enable after reset settles
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  delay(10);
}

// ---------------------------------------------------------------
// Parallel load — /PE LOW, data stable, then clock
// CEP and CET are driven LOW first to prevent any accidental
// count if /PE doesn't reach the chip
// ---------------------------------------------------------------
void parallelLoad(uint8_t val) {
  // Step 1: disable count enables
  digitalWrite(PIN_CEP, LOW);
  digitalWrite(PIN_CET, LOW);
  delay(20);
  // Step 2: apply data
  for (int i = 0; i < 4; i++) {
    digitalWrite(PIN_P[i], (val >> i) & 1);
  }
  delay(20);
  // Step 3: assert /PE
  digitalWrite(PIN_PE, LOW);
  delay(30);
  // Step 4: clock — load happens on rising edge
  digitalWrite(PIN_CP, LOW);
  delay(20);
  digitalWrite(PIN_CP, HIGH);
  delay(20);
  digitalWrite(PIN_CP, LOW);
  delay(20);
  // Step 5: release /PE before re-enabling count
  digitalWrite(PIN_PE, HIGH);
  delay(20);
  // Step 6: restore count enables
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  delay(10);
}

// ---------------------------------------------------------------
// Count mode pulse — CEP=H, CET=H, /PE=H, /MR=H
// ---------------------------------------------------------------
void countPulse() {
  digitalWrite(PIN_MR,  HIGH);
  digitalWrite(PIN_PE,  HIGH);
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  delay(10);
  pulseClock();
}

// ---------------------------------------------------------------
// Print binary 4-bit value
// ---------------------------------------------------------------
void printBin4(uint8_t val) {
  for (int i = 3; i >= 0; i--) Serial.print((val >> i) & 1);
}

// ---------------------------------------------------------------
// Print result row
// ---------------------------------------------------------------
bool printResult(const char* phase, const char* mode,
                 uint8_t actual, uint8_t expected,
                 int actualTC, int expectedTC) {
  bool ok = (actual == expected) && (actualTC == expectedTC);
  if (ok) totalPass++; else totalFail++;

  Serial.print(F("  "));
  int plen = strlen(phase);
  Serial.print(phase);
  for (int i = plen; i < 9; i++) Serial.print(' ');
  Serial.print(F("  "));
  int mlen = strlen(mode);
  Serial.print(mode);
  for (int i = mlen; i < 24; i++) Serial.print(' ');
  Serial.print(F("  Q="));
  printBin4(actual);
  Serial.print(F(" ("));
  if (actual < 10) Serial.print(' ');
  Serial.print(actual);
  Serial.print(F(") TC="));
  Serial.print(actualTC);
  Serial.print(F("  exp="));
  printBin4(expected);
  Serial.print(F(" ("));
  if (expected < 10) Serial.print(' ');
  Serial.print(expected);
  Serial.print(F(") TC="));
  Serial.print(expectedTC);
  Serial.print(F("  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));
  return ok;
}

void printSep() {
  Serial.println(F("  -------------------------------------------------------------------------------"));
}

void printHeader() {
  Serial.println(F("  Phase     Mode                      Q=xxxx (n) TC=x  exp=xxxx (n) TC=x  result"));
  printSep();
}

// ---------------------------------------------------------------
// Phase 1: Async Reset
// ---------------------------------------------------------------
void testAsyncReset() {
  Serial.println(F("\n--- Phase 1: Async Reset (/MR) ---"));
  Serial.println(F("  /MR LOW clears Q immediately, no clock needed"));
  printHeader();

  // Load a known non-zero value
  parallelLoad(0b1010);
  printResult("Setup", "Load 1010 before reset ", readCount(), 10, readTC(), 0);

  // Assert /MR — clears immediately
  digitalWrite(PIN_CEP, LOW);
  digitalWrite(PIN_CET, LOW);
  digitalWrite(PIN_MR, LOW);
  delay(30);
  printResult("Async", "/MR asserted (no clock)", readCount(), 0, readTC(), 0);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
  printResult("Async", "/MR released           ", readCount(), 0, readTC(), 0);

  // /MR must override clock
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  digitalWrite(PIN_MR, LOW);
  delay(20);
  pulseClock();
  printResult("Async", "/MR LOW with CLK pulse ", readCount(), 0, readTC(), 0);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Phase 2: Parallel Load
// ---------------------------------------------------------------
void testParallelLoad() {
  Serial.println(F("\n--- Phase 2: Parallel Load (/PE) ---"));
  Serial.println(F("  /PE LOW loads P0-P3 to Q on rising clock edge"));
  printHeader();

  asyncReset();

  uint8_t testVals[] = {0b0000, 0b0001, 0b0101, 0b1010,
                        0b1111, 0b1001, 0b0110, 0b1100};
  const char* labels[] = {
    "Load 0000 ( 0)         ",
    "Load 0001 ( 1)         ",
    "Load 0101 ( 5)         ",
    "Load 1010 (10)         ",
    "Load 1111 (15)         ",
    "Load 1001 ( 9)         ",
    "Load 0110 ( 6)         ",
    "Load 1100 (12)         "
  };

  for (int i = 0; i < 8; i++) {
    uint8_t val = testVals[i];
    parallelLoad(val);
    // TC is HIGH when count=15 and CET=HIGH (which is restored after load)
    int expTC = (val == 15) ? 1 : 0;
    printResult("Load", labels[i], readCount(), val, readTC(), expTC);
  }
}

// ---------------------------------------------------------------
// Phase 3: Count Sequence 0-15 and wrap
// ---------------------------------------------------------------
void testCountSequence() {
  Serial.println(F("\n--- Phase 3: Count Sequence (0 -> 15 -> 0) ---"));
  Serial.println(F("  CEP=H CET=H /PE=H: increments on each rising edge"));
  Serial.println(F("  TC must be HIGH only at count=15"));
  printHeader();

  asyncReset();
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  digitalWrite(PIN_PE,  HIGH);

  for (uint8_t expected = 0; expected <= 16; expected++) {
    uint8_t expCount = expected % 16;
    int expTC = (expCount == 15) ? 1 : 0;
    char modeBuf[26];
    snprintf(modeBuf, sizeof(modeBuf), "Count -> %2d            ", expCount);
    printResult("Count", modeBuf, readCount(), expCount, readTC(), expTC);
    if (expected < 16) countPulse();
  }
}

// ---------------------------------------------------------------
// Phase 4: Hold modes
// ---------------------------------------------------------------
void testHold() {
  Serial.println(F("\n--- Phase 4: Hold Modes ---"));
  Serial.println(F("  CEP=L or CET=L: clock must not increment counter"));
  printHeader();

  // Load 7
  asyncReset();
  parallelLoad(0b0111);
  printResult("Hold", "Load 7 for hold test   ", readCount(), 7, readTC(), 0);

  // Hold with CEP=LOW — clock 3 times, must stay at 7
  digitalWrite(PIN_CEP, LOW);
  digitalWrite(PIN_CET, HIGH);
  digitalWrite(PIN_PE,  HIGH);
  delay(20);
  for (int i = 0; i < 3; i++) {
    pulseClock();
    printResult("Hold", "CEP=L (hold, no count) ", readCount(), 7, readTC(), 0);
  }

  // Hold with CET=LOW — clock 3 times, must stay at 7
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, LOW);
  delay(20);
  for (int i = 0; i < 3; i++) {
    pulseClock();
    printResult("Hold", "CET=L (hold, no count) ", readCount(), 7, readTC(), 0);
  }

  // Resume counting from 7
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  delay(20);
  countPulse();
  printResult("Hold", "Resume count (7->8)    ", readCount(), 8, readTC(), 0);
}

// ---------------------------------------------------------------
// Phase 5: Terminal Count
// ---------------------------------------------------------------
void testTerminalCount() {
  Serial.println(F("\n--- Phase 5: Terminal Count (TC) Output ---"));
  Serial.println(F("  TC=HIGH only at count=15 with CET=HIGH"));
  printHeader();

  asyncReset();
  parallelLoad(14);
  printResult("TC", "Load 14                ", readCount(), 14, readTC(), 0);

  // Count 14->15, TC must go HIGH
  digitalWrite(PIN_CEP, HIGH);
  digitalWrite(PIN_CET, HIGH);
  countPulse();
  printResult("TC", "Count 14->15 TC=H      ", readCount(), 15, readTC(), 1);

  // TC must be LOW when CET=LOW at count=15
  digitalWrite(PIN_CET, LOW);
  delay(20);
  printResult("TC", "CET=L at 15 TC=L       ", readCount(), 15, readTC(), 0);
  digitalWrite(PIN_CET, HIGH);
  delay(20);

  // Wrap to 0 — TC goes LOW
  countPulse();
  printResult("TC", "Count 15->0 TC=L       ", readCount(), 0, readTC(), 0);

  // TC must stay LOW for counts 1-14
  Serial.println(F("  -- TC must stay LOW for counts 1-14 --"));
  bool tcClean = true;
  for (uint8_t n = 1; n <= 14; n++) {
    countPulse();
    uint8_t q = readCount();
    int tc = readTC();
    if (tc != 0 || q != n) {
      char modeBuf[26];
      snprintf(modeBuf, sizeof(modeBuf), "Count %2d TC must=0     ", n);
      printResult("TC", modeBuf, q, n, tc, 0);
      tcClean = false;
    }
  }
  if (tcClean) {
    Serial.println(F("  TC        Counts 1-14 all TC=0       (14 checks all PASS)"));
    totalPass += 14;
  }
}

// ---------------------------------------------------------------
// Phase 6: Load overrides count enable
// ---------------------------------------------------------------
void testLoadOverridesCount() {
  Serial.println(F("\n--- Phase 6: Load Overrides Count Enable ---"));
  Serial.println(F("  /PE=LOW loads data even when CEP=L or CET=L"));
  printHeader();

  asyncReset();

  // Load 12 with CEP=LOW — use parallelLoad which already sets CEP/CET LOW
  parallelLoad(0b1100);
  printResult("Load>En", "Load 12 CEP=L CET=L    ", readCount(), 12, readTC(), 0);

  // Load 3 — verify previous load was correct before next
  parallelLoad(0b0011);
  printResult("Load>En", "Load  3                ", readCount(), 3, readTC(), 0);

  // Load 9
  parallelLoad(0b1001);
  printResult("Load>En", "Load  9                ", readCount(), 9, readTC(), 0);

  // Now verify counting still works after loads
  countPulse();
  printResult("Load>En", "Count after load (9->10)", readCount(), 10, readTC(), 0);
  countPulse();
  printResult("Load>En", "Count after load (10->11)", readCount(), 11, readTC(), 0);
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Set safe states BEFORE enabling outputs to avoid glitches
  digitalWrite(PIN_MR,  HIGH);  // not resetting
  digitalWrite(PIN_CP,  LOW);
  digitalWrite(PIN_CEP, LOW);   // hold initially
  digitalWrite(PIN_CET, LOW);   // hold initially
  digitalWrite(PIN_PE,  HIGH);  // count mode (not loading)
  for (int i = 0; i < 4; i++) digitalWrite(PIN_P[i], LOW);

  pinMode(PIN_MR,  OUTPUT);
  pinMode(PIN_CP,  OUTPUT);
  pinMode(PIN_CEP, OUTPUT);
  pinMode(PIN_CET, OUTPUT);
  pinMode(PIN_PE,  OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_P[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_Q[i], INPUT);
  pinMode(PIN_TC, INPUT);

  delay(200);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS161N 4-Bit Binary Counter Tester"));
  Serial.println(F("========================================"));

  testAsyncReset();
  testParallelLoad();
  testCountSequence();
  testHold();
  testTerminalCount();
  testLoadOverridesCount();

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
