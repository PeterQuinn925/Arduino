// ============================================================
//  74LS166N Parallel-Load 8-Bit Shift Register Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Control inputs:
//   D2  (SH/LD) [15]  Shift/Load: HIGH=shift mode, LOW=load mode
//   D3  (CLK)   [7]   Clock, rising edge triggered
//   D4  (CLKINH)[6]   Clock Inhibit: HIGH=inhibit clock, LOW=allow clock
//   D5  (/CLR)  [9]   Async Clear, active LOW (HIGH=inactive)
//   D6  (SER)   [1]   Serial input (used during shift mode)
//
//  Parallel data inputs (loaded when SH/LD=LOW):
//   D7  (A)     [2]   Parallel bit 0 (first to shift out)
//   D8  (B)     [3]   Parallel bit 1
//   D9  (C)     [4]   Parallel bit 2
//   D10 (D)     [5]   Parallel bit 3
//   D11 (E)     [10]  Parallel bit 4
//   D12 (F)     [11]  Parallel bit 5
//   D13 (G)     [12]  Parallel bit 6
//   A0  (H)     [13]  Parallel bit 7 (last to shift out)
//
//  Output:
//   A1  (QH)    [14]  Serial output (MSB shifts out first)
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  How it works:
//   1. Load: set SH/LD=LOW, apply data to A-H, pulse CLK.
//      All 8 bits loaded simultaneously on rising edge.
//   2. Shift: set SH/LD=HIGH, pulse CLK 8 times.
//      QH outputs A on clock 1, B on clock 2 ... H on clock 8.
//   3. Clear: /CLR=LOW clears all bits immediately (no clock).
//   4. Inhibit: CLKINH=HIGH blocks the clock entirely.
//
//  Function table:
//   /CLR  SH/LD  CLKINH  CLK  | Action
//   L     X      X       X    | Async clear (QH=0 immediately)
//   H     L      L       ↑    | Parallel load A-H
//   H     H      L       ↑    | Shift: QH=QGn, serial in at A
//   H     X      H       X    | Clock inhibited (hold)
// ============================================================

// --- Control pins ---
const int PIN_SHLD  = 2;   // HIGH=shift, LOW=load
const int PIN_CLK   = 3;
const int PIN_CLKINH= 4;   // HIGH=inhibit
const int PIN_CLR   = 5;   // active LOW
const int PIN_SER   = 6;   // serial input

// --- Parallel input pins A-H ---
const int PIN_PAR[8] = {7, 8, 9, 10, 11, 12, 13, A0};  // A..H

// --- Serial output ---
const int PIN_QH = A1;

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Rising edge clock pulse
// ---------------------------------------------------------------
void pulseClock() {
  digitalWrite(PIN_CLK, LOW);
  delay(10);
  digitalWrite(PIN_CLK, HIGH);   // rising edge
  delay(10);
  digitalWrite(PIN_CLK, LOW);
  delay(10);
}

// ---------------------------------------------------------------
// Parallel load: apply 8-bit value to A-H, pulse clock
// SH/LD must be LOW before calling
// ---------------------------------------------------------------
void parallelLoad(uint8_t val) {
  // Inhibit clock while setting up
  digitalWrite(PIN_CLKINH, HIGH);
  delay(10);
  // Apply data A (bit0) through H (bit7)
  for (int i = 0; i < 8; i++) {
    digitalWrite(PIN_PAR[i], (val >> i) & 1);
  }
  delay(20);
  // Assert load mode
  digitalWrite(PIN_SHLD, LOW);
  delay(20);
  // Allow clock and pulse
  digitalWrite(PIN_CLKINH, LOW);
  delay(10);
  pulseClock();
  // Return to shift mode
  digitalWrite(PIN_SHLD, HIGH);
  delay(10);
}

// ---------------------------------------------------------------
// Shift out all 8 bits, return them as a byte (A=bit7 first out)
// Reads QH after each clock pulse
// SH/LD must be HIGH before calling
// ---------------------------------------------------------------
uint8_t shiftOut8(uint8_t serialIn) {
  uint8_t result = 0;
  digitalWrite(PIN_SHLD,   HIGH);
  digitalWrite(PIN_CLKINH, LOW);
  digitalWrite(PIN_SER, (serialIn >> 7) & 1);
  delay(10);
  for (int i = 0; i < 8; i++) {
    // Set up serial input for this clock
    // SER feeds into position A, shifting everything right
    // For simplicity we keep SER LOW (0) during shift-out tests
    // unless explicitly testing serial input
    pulseClock();
    int bit = digitalRead(PIN_QH);
    result = (result << 1) | bit;
  }
  return result;
}

// ---------------------------------------------------------------
// Async clear
// ---------------------------------------------------------------
void asyncClear() {
  digitalWrite(PIN_CLR, LOW);
  delay(30);
  digitalWrite(PIN_CLR, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Print 8-bit value as binary and decimal
// ---------------------------------------------------------------
void printBin8(uint8_t val) {
  for (int i = 7; i >= 0; i--) Serial.print((val >> i) & 1);
  Serial.print(F(" (0x"));
  if (val < 0x10) Serial.print('0');
  Serial.print(val, HEX);
  Serial.print(F(")"));
}

// ---------------------------------------------------------------
// Print result row
// ---------------------------------------------------------------
bool printResult(const char* phase, const char* mode,
                 uint8_t actual, uint8_t expected) {
  bool ok = (actual == expected);
  if (ok) totalPass++; else totalFail++;

  Serial.print(F("  "));
  int plen = strlen(phase);
  Serial.print(phase);
  for (int i = plen; i < 8; i++) Serial.print(' ');
  Serial.print(F("  "));
  int mlen = strlen(mode);
  Serial.print(mode);
  for (int i = mlen; i < 26; i++) Serial.print(' ');
  Serial.print(F("  got="));
  printBin8(actual);
  Serial.print(F("  exp="));
  printBin8(expected);
  Serial.print(F("  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));
  return ok;
}

void printSep() {
  Serial.println(F("  ---------------------------------------------------------------------------------------------"));
}

void printHeader() {
  Serial.println(F("  Phase     Mode                        got=xxxxxxxx       exp=xxxxxxxx       result"));
  printSep();
}

// ---------------------------------------------------------------
// Phase 1: Async Clear
// /CLR LOW must clear QH immediately, no clock needed
// After 8 shift pulses, all bits should be 0
// ---------------------------------------------------------------
void testAsyncClear() {
  Serial.println(F("\n--- Phase 1: Async Clear (/CLR) ---"));
  Serial.println(F("  /CLR LOW clears all bits to 0 immediately, no clock needed"));
  Serial.println(F("  After 8 shift pulses following a clear, QH output must be all 0s"));
  printHeader();

  // Load all 1s first
  parallelLoad(0xFF);
  // Clear without clock
  digitalWrite(PIN_CLR, LOW);
  delay(30);
  // Shift out — all bits should be 0
  digitalWrite(PIN_CLR, HIGH);
  delay(10);
  digitalWrite(PIN_SHLD, HIGH);
  uint8_t result = shiftOut8(0);
  printResult("Clear", "Load 0xFF then /CLR=L  ", result, 0x00);

  // Verify /CLR overrides clock — load 0xFF, assert /CLR, pulse clock, shift out
  parallelLoad(0xFF);
  digitalWrite(PIN_CLR, LOW);
  delay(20);
  pulseClock();   // clock while CLR asserted — must not shift
  digitalWrite(PIN_CLR, HIGH);
  delay(10);
  result = shiftOut8(0);
  printResult("Clear", "CLR overrides CLK      ", result, 0x00);

  // Verify /CLR overrides SH/LD=LOW (load mode) too
  parallelLoad(0xFF);
  digitalWrite(PIN_CLR,  LOW);
  digitalWrite(PIN_SHLD, LOW);   // load mode
  delay(20);
  pulseClock();
  digitalWrite(PIN_CLR,  HIGH);
  digitalWrite(PIN_SHLD, HIGH);
  delay(10);
  result = shiftOut8(0);
  printResult("Clear", "CLR overrides load mode", result, 0x00);
}

// ---------------------------------------------------------------
// Phase 2: Parallel Load and Shift Out
// Load a known byte, shift it out, verify QH sequence
// ---------------------------------------------------------------
void testParallelLoad() {
  Serial.println(F("\n--- Phase 2: Parallel Load + Shift Out ---"));
  Serial.println(F("  Load 8-bit value via A-H, shift out via QH, verify all 8 bits"));
  printHeader();

  uint8_t testVals[] = {
    0b10101010,   // alternating
    0b01010101,   // alternating inverse
    0b11110000,   // upper nibble
    0b00001111,   // lower nibble
    0b11111111,   // all ones
    0b00000000,   // all zeros
    0b10000001,   // ends only
    0b01111110,   // middle only
    0b10110100,   // random
    0b00110101    // random
  };

  const char* labels[] = {
    "Load 10101010         ",
    "Load 01010101         ",
    "Load 11110000         ",
    "Load 00001111         ",
    "Load 11111111         ",
    "Load 00000000         ",
    "Load 10000001         ",
    "Load 01111110         ",
    "Load 10110100         ",
    "Load 00110101         "
  };

  for (int i = 0; i < 10; i++) {
    asyncClear();
    parallelLoad(testVals[i]);
    digitalWrite(PIN_SER, LOW);
    uint8_t result = shiftOut8(0);
    printResult("Load", labels[i], result, testVals[i]);
  }
}

// ---------------------------------------------------------------
// Phase 3: Serial Shift In
// Clear register, then shift in a known pattern serially via SER
// After 8 clocks the pattern should appear at QH
// ---------------------------------------------------------------
void testSerialShift() {
  Serial.println(F("\n--- Phase 3: Serial Input ---"));
  Serial.println(F("  Shift 8 bits in via SER pin, verify they appear at QH"));
  Serial.println(F("  Bits shift MSB first: bit 7 in on clock 1, bit 0 in on clock 8"));
  printHeader();

  uint8_t testVals[] = {
    0b10101010,
    0b01010101,
    0b11001100,
    0b11111111,
    0b00000000,
    0b10000000,
    0b00000001
  };

  const char* labels[] = {
    "Serial in 10101010    ",
    "Serial in 01010101    ",
    "Serial in 11001100    ",
    "Serial in 11111111    ",
    "Serial in 00000000    ",
    "Serial in 10000000    ",
    "Serial in 00000001    "
  };

  for (int t = 0; t < 7; t++) {
    uint8_t val = testVals[t];
    asyncClear();

    // Enter shift mode
    digitalWrite(PIN_SHLD,   HIGH);
    digitalWrite(PIN_CLKINH, LOW);
    delay(10);

    // Shift in 8 bits MSB first via SER, collect QH output
    uint8_t shiftedOut = 0;
    for (int i = 7; i >= 0; i--) {
      digitalWrite(PIN_SER, (val >> i) & 1);
      delay(10);
      pulseClock();
      int bit = digitalRead(PIN_QH);
      shiftedOut = (shiftedOut << 1) | bit;
    }
    // After 8 clocks the original register (all 0s from clear)
    // has been pushed out and the new serial data is inside.
    // shiftedOut = the 8 zeros that were cleared out (expect 0x00)
    // Now shift 8 more times with SER=0 to read back what we shifted in
    uint8_t readBack = 0;
    digitalWrite(PIN_SER, LOW);
    for (int i = 0; i < 8; i++) {
      pulseClock();
      int bit = digitalRead(PIN_QH);
      readBack = (readBack << 1) | bit;
    }
    printResult("Serial", labels[t], readBack, val);
  }
}

// ---------------------------------------------------------------
// Phase 4: Clock Inhibit
// CLKINH=HIGH must block clock — register must not change
// ---------------------------------------------------------------
void testClockInhibit() {
  Serial.println(F("\n--- Phase 4: Clock Inhibit (CLKINH=HIGH) ---"));
  Serial.println(F("  CLKINH=HIGH blocks clock — register must not shift or load"));
  printHeader();

  // Load a known value
  asyncClear();
  parallelLoad(0b10110011);

  // Inhibit clock and try to shift — output must not change
  digitalWrite(PIN_SHLD,   HIGH);
  digitalWrite(PIN_CLKINH, HIGH);   // inhibit
  delay(10);

  // Pulse clock 3 times with inhibit HIGH — should have no effect
  for (int i = 0; i < 3; i++) pulseClock();

  // Now allow clock and shift out — should still be original value
  digitalWrite(PIN_CLKINH, LOW);
  delay(10);
  uint8_t result = shiftOut8(0);
  printResult("Inhibit", "3 CLK pulses inhibited ", result, 0b10110011);

  // Also verify inhibit blocks parallel load
  asyncClear();
  parallelLoad(0b11001100);   // load a known value
  // Now inhibit and try to load different value
  digitalWrite(PIN_CLKINH, HIGH);
  for (int i = 0; i < 8; i++) digitalWrite(PIN_PAR[i], 1);  // try to load 0xFF
  digitalWrite(PIN_SHLD, LOW);
  pulseClock();   // inhibited — load must not happen
  digitalWrite(PIN_SHLD,   HIGH);
  digitalWrite(PIN_CLKINH, LOW);
  delay(10);
  result = shiftOut8(0);
  printResult("Inhibit", "Load blocked by CLKINH ", result, 0b11001100);
}

// ---------------------------------------------------------------
// Phase 5: Walking 1 test
// Load each bit position with a 1 and all others 0,
// verify only the correct bit appears at QH in the correct clock cycle
// ---------------------------------------------------------------
void testWalkingOne() {
  Serial.println(F("\n--- Phase 5: Walking 1 (bit isolation) ---"));
  Serial.println(F("  Load a single 1 bit in each position, shift out, verify position"));
  printHeader();

  for (int bit = 0; bit < 8; bit++) {
    uint8_t val = (1 << bit);
    asyncClear();
    parallelLoad(val);
    digitalWrite(PIN_SER, LOW);
    uint8_t result = shiftOut8(0);

    char modeBuf[28];
    snprintf(modeBuf, sizeof(modeBuf), "Walking 1 at bit %d      ", bit);
    printResult("Walk1", modeBuf, result, val);
  }
}

// ---------------------------------------------------------------
// Phase 6: Walking 0 test
// ---------------------------------------------------------------
void testWalkingZero() {
  Serial.println(F("\n--- Phase 6: Walking 0 (bit isolation) ---"));
  Serial.println(F("  Load a single 0 bit in each position (all others 1), shift out"));
  printHeader();

  for (int bit = 0; bit < 8; bit++) {
    uint8_t val = ~(1 << bit) & 0xFF;
    asyncClear();
    parallelLoad(val);
    digitalWrite(PIN_SER, LOW);
    uint8_t result = shiftOut8(0);

    char modeBuf[28];
    snprintf(modeBuf, sizeof(modeBuf), "Walking 0 at bit %d      ", bit);
    printResult("Walk0", modeBuf, result, val);
  }
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Safe states before enabling outputs
  digitalWrite(PIN_CLR,    HIGH);   // not clearing
  digitalWrite(PIN_CLK,    LOW);
  digitalWrite(PIN_CLKINH, HIGH);   // inhibit clock initially
  digitalWrite(PIN_SHLD,   HIGH);   // shift mode
  digitalWrite(PIN_SER,    LOW);
  for (int i = 0; i < 8; i++) digitalWrite(PIN_PAR[i], LOW);

  pinMode(PIN_CLR,    OUTPUT);
  pinMode(PIN_CLK,    OUTPUT);
  pinMode(PIN_CLKINH, OUTPUT);
  pinMode(PIN_SHLD,   OUTPUT);
  pinMode(PIN_SER,    OUTPUT);
  for (int i = 0; i < 8; i++) pinMode(PIN_PAR[i], OUTPUT);
  pinMode(PIN_QH, INPUT);

  // Initial clear
  asyncClear();

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS166N 8-Bit Shift Register Tester"));
  Serial.println(F("========================================"));

  testAsyncClear();
  testParallelLoad();
  testSerialShift();
  testClockInhibit();
  testWalkingOne();
  testWalkingZero();

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
