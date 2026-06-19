// ============================================================
//  74LS195N 4-Bit Parallel-Access Universal Shift Register
//  Tester — Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
// Claude's pinout appears wrong.
/* The pinout that I see on the data sheet is
Pin 1 MR Master reset input   D2
Pin 2 J First stage J input (active HIGH) D5
Pin 3 KB First Stage K Input (active LOW) D6
Pin 4 P0 Parallel data input  D7
Pin 5 P1  D8
Pin 6 P2  D9
Pin 7 P3  D10
Pin 8 GND
Pin 9 PE Parallel Enable Input (active LOW) D4
Pin 10 CP Clock Pulse Input (Active Rising Edge)  D3
Pin 11 Q3B Complementary Last Stage Output (Active LOW) D11
Pin 12 Q3 Parallel Output A3
Pin 13 Q2 A2
Pin 14 Q1 A1
Pin 15 Q0 A0
Pin 16 VCC
*/
// OLD/BAD Wiring - ignore
//  Control inputs:
//   D2  (MR)  [7]   Master Reset, active LOW (HIGH=inactive)
//   D3  (CP)  [6]   Clock, active HIGH-going edge
//   D4  (PE)  [10]  Parallel Enable, active LOW
//                    LOW  = parallel load mode (P0-P3 -> Q0-Q3)
//                    HIGH = shift mode (J/K feed Q0 on each clock)
//   D5  (J)   [1]   First-stage J input (active HIGH) — shift mode only
//   D6  (K)   [9]   First-stage K input (active LOW)  — shift mode only
//
//  Parallel data inputs (loaded when PE=LOW):
//   D7  (P0)  [2]   Data bit 0
//   D8  (P1)  [3]   Data bit 1
//   D9  (P2)  [4]   Data bit 2
//   D10 (P3)  [5]   Data bit 3
//
//  Outputs:
//   A0  (Q0)  [11]  Output bit 0
//   A1  (Q1)  [12]  Output bit 1
//   A2  (Q2)  [13]  Output bit 2
//   A3  (Q3)  [14]  Output bit 3
//   D11 (/Q3) [15]  Complement of Q3 (always opposite of Q3)
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  How shift mode works (PE=HIGH):
//   J=H, K=L  -> Q0 becomes 1 on next clock (SET)
//   J=L, K=H  -> Q0 becomes 0 on next clock (RESET)
//   J=L, K=L  -> Q0 becomes 0 on next clock (same as reset, per datasheet)
//   J=H, K=H  -> Q0 TOGGLES on next clock
//   Existing bits shift right: Q0->Q1->Q2->Q3 on each clock
//
//  Function table summary:
//   MR   PE   CP   | Action
//   L    X    X    | Async clear, all Q=0, overrides everything
//   H    L    rise | Parallel load P0-P3 -> Q0-Q3
//   H    H    rise | Shift: Q0=f(J,K), Q1=oldQ0, Q2=oldQ1, Q3=oldQ2
// ============================================================

// --- Control pins --- REDO with what I see on the DS
const int PIN_MR = 2;    // active LOW
const int PIN_CP = 3;
const int PIN_PE = 4;    // active LOW
const int PIN_J  = 5;
const int PIN_K  = 6;

// --- Parallel data input pins ---
const int PIN_P[4] = {7, 8, 9, 10};   // P0..P3

// --- Output pins ---
const int PIN_Q[4] = {A0, A1, A2, A3};  // Q0..Q3
const int PIN_Q3B  = 11;                // /Q3

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Rising edge clock pulse
// ---------------------------------------------------------------
void pulseClock() {
  digitalWrite(PIN_CP, LOW);
  delay(15);
  digitalWrite(PIN_CP, HIGH);   // rising edge
  delay(15);
  digitalWrite(PIN_CP, LOW);
  delay(15);
}

// ---------------------------------------------------------------
// Async master reset — MR LOW clears all Q immediately
// ---------------------------------------------------------------
void asyncReset() {
  digitalWrite(PIN_MR, LOW);
  delay(30);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Apply parallel data P0-P3
// ---------------------------------------------------------------
void applyData(uint8_t val) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(PIN_P[i], (val >> i) & 1);
  }
}

// ---------------------------------------------------------------
// Parallel load: PE LOW, apply data, clock, restore PE HIGH
// ---------------------------------------------------------------
void parallelLoad(uint8_t val) {
  digitalWrite(PIN_PE, LOW);
  applyData(val);
  delay(20);
  pulseClock();
  digitalWrite(PIN_PE, HIGH);
  delay(10);
}

// ---------------------------------------------------------------
// Shift one bit in via J/K. PE must already be HIGH.
// jVal/kVal: 1 or 0 for J and K inputs
// ---------------------------------------------------------------
void shiftWithJK(int jVal, int kVal) {
  digitalWrite(PIN_J, jVal);
  digitalWrite(PIN_K, kVal);
  delay(15);
  pulseClock();
}

// ---------------------------------------------------------------
// Read Q0-Q3 as a 4-bit value
// ---------------------------------------------------------------
uint8_t readQ() {
  delay(10);
  uint8_t val = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(PIN_Q[i])) val |= (1 << i);
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
// Print result row, checking Q and /Q3 complement
// ---------------------------------------------------------------
bool printResult(const char* phase, const char* mode, uint8_t expectedQ) {
  uint8_t actualQ = readQ();
  int actualQ3B   = digitalRead(PIN_Q3B);
  int expectedQ3  = (expectedQ >> 3) & 1;
  int expectedQ3B = !expectedQ3;

  bool ok = (actualQ == expectedQ) && (actualQ3B == expectedQ3B);
  if (ok) totalPass++; else totalFail++;

  Serial.print(F("  "));
  int plen = strlen(phase);
  Serial.print(phase);
  for (int i = plen; i < 8; i++) Serial.print(' ');
  Serial.print(F("  "));
  int mlen = strlen(mode);
  Serial.print(mode);
  for (int i = mlen; i < 26; i++) Serial.print(' ');
  Serial.print(F("  Q="));
  printBin4(actualQ);
  Serial.print(F(" /Q3="));
  Serial.print(actualQ3B);
  Serial.print(F("  exp Q="));
  printBin4(expectedQ);
  Serial.print(F(" /Q3="));
  Serial.print(expectedQ3B);
  Serial.print(F("  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));
  return ok;
}

void printSep() {
  Serial.println(F("  -----------------------------------------------------------------------"));
}

void printHeader() {
  Serial.println(F("  Phase     Mode                        Q=xxxx /Q3=x  exp Q=xxxx /Q3=x  result"));
  printSep();
}

// ---------------------------------------------------------------
// Phase 1: Async Master Reset
// ---------------------------------------------------------------
void testAsyncReset() {
  Serial.println(F("\n--- Phase 1: Async Master Reset (MR) ---"));
  Serial.println(F("  MR LOW clears all Q to 0 immediately, no clock required"));
  printHeader();

  parallelLoad(0b1111);
  printResult("Setup", "Load 1111 before reset    ", 0b1111);

  digitalWrite(PIN_MR, LOW);
  delay(30);
  printResult("Async", "MR asserted (no clock)    ", 0b0000);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
  printResult("Async", "MR released                ", 0b0000);

  // MR overrides clock
  parallelLoad(0b1111);
  digitalWrite(PIN_MR, LOW);
  delay(20);
  pulseClock();
  printResult("Async", "MR=L during CLK pulse      ", 0b0000);
  digitalWrite(PIN_MR, HIGH);
  delay(20);
}

// ---------------------------------------------------------------
// Phase 2: Parallel Load
// ---------------------------------------------------------------
void testParallelLoad() {
  Serial.println(F("\n--- Phase 2: Parallel Load (PE=LOW) ---"));
  Serial.println(F("  Data on P0-P3 loaded to Q0-Q3 on rising clock edge"));
  printHeader();

  asyncReset();

  uint8_t testVals[] = {0,1,5,10,15,9,6,12,3,8};
  const char* labels[] = {
    "Load 0000 ( 0)            ",
    "Load 0001 ( 1)            ",
    "Load 0101 ( 5)            ",
    "Load 1010 (10)            ",
    "Load 1111 (15)            ",
    "Load 1001 ( 9)            ",
    "Load 0110 ( 6)            ",
    "Load 1100 (12)            ",
    "Load 0011 ( 3)            ",
    "Load 1000 ( 8)            "
  };

  for (int i = 0; i < 10; i++) {
    parallelLoad(testVals[i]);
    printResult("Load", labels[i], testVals[i]);
  }
}

// ---------------------------------------------------------------
// Phase 3: Shift Right with J/K Set and Reset
// PE=HIGH, verify Q0=f(J,K) and existing bits shift right
// ---------------------------------------------------------------
void testShiftSetReset() {
  Serial.println(F("\n--- Phase 3: Shift with J/K Set/Reset ---"));
  Serial.println(F("  PE=HIGH; J=H,K=L sets Q0=1; J=L,K=H resets Q0=0"));
  Serial.println(F("  Existing bits shift right: Q0->Q1->Q2->Q3"));
  printHeader();

  asyncReset();
  digitalWrite(PIN_PE, HIGH);

  // Shift in 1,0,1,1 (J=H/K=L sets 1, J=L/K=H sets 0)
  shiftWithJK(HIGH, LOW);   // Q0=1
  printResult("Shift", "J=H K=L shift in 1 (Q0)   ", 0b0001);

  shiftWithJK(LOW, HIGH);   // Q0=0, old Q0(1) -> Q1
  printResult("Shift", "J=L K=H shift in 0 (Q1<-1)", 0b0010);

  shiftWithJK(HIGH, LOW);   // Q0=1, shift
  printResult("Shift", "J=H K=L shift in 1 (Q2<-1)", 0b0101);

  shiftWithJK(HIGH, LOW);   // Q0=1, shift
  printResult("Shift", "J=H K=L shift in 1 (Q3<-1)", 0b1011);
}

// ---------------------------------------------------------------
// Phase 4: Shift with Toggle (J=H, K=H)
// ---------------------------------------------------------------
void testShiftToggle() {
  Serial.println(F("\n--- Phase 4: Shift with Toggle (J=H K=H) ---"));
  Serial.println(F("  J=H K=H makes Q0 toggle each clock instead of set/reset"));
  printHeader();

  asyncReset();
  digitalWrite(PIN_PE, HIGH);

  // Start with Q0=0 known via reset; first toggle should set Q0=1
  shiftWithJK(HIGH, HIGH);   // Q0 toggles 0->1
  printResult("Toggle", "Toggle 1: Q0 0->1          ", 0b0001);

  shiftWithJK(HIGH, HIGH);   // Q0 toggles 1->0, old Q0(1)->Q1
  printResult("Toggle", "Toggle 2: Q0 1->0          ", 0b0010);

  shiftWithJK(HIGH, HIGH);   // Q0 toggles 0->1, shift
  printResult("Toggle", "Toggle 3: Q0 0->1          ", 0b0101);

  shiftWithJK(HIGH, HIGH);   // Q0 toggles 1->0, shift
  printResult("Toggle", "Toggle 4: Q0 1->0          ", 0b1010);
}

// ---------------------------------------------------------------
// Phase 5: Walking 1 through shift register
// Load 0001 then shift with J=L,K=H (shift in 0s) to walk the 1
// ---------------------------------------------------------------
void testWalkingOne() {
  Serial.println(F("\n--- Phase 5: Walking 1 Through Register ---"));
  Serial.println(F("  Load single 1 at Q0, shift 0s in, watch it walk to Q3"));
  printHeader();

  asyncReset();
  parallelLoad(0b0001);
  printResult("Walk1", "Initial load 0001          ", 0b0001);

  digitalWrite(PIN_PE, HIGH);
  shiftWithJK(LOW, HIGH);   // shift in 0
  printResult("Walk1", "After shift: 1 moves to Q1 ", 0b0010);

  shiftWithJK(LOW, HIGH);
  printResult("Walk1", "After shift: 1 moves to Q2 ", 0b0100);

  shiftWithJK(LOW, HIGH);
  printResult("Walk1", "After shift: 1 moves to Q3 ", 0b1000);

  shiftWithJK(LOW, HIGH);
  printResult("Walk1", "After shift: 1 shifted out ", 0b0000);
}

// ---------------------------------------------------------------
// Phase 6: PE overrides — verify shift mode does not affect
// register when PE=LOW (load mode) regardless of J/K
// ---------------------------------------------------------------
void testPEOverride() {
  Serial.println(F("\n--- Phase 6: PE Mode Override ---"));
  Serial.println(F("  PE=LOW ignores J/K; only P0-P3 affects Q on clock"));
  printHeader();

  asyncReset();
  parallelLoad(0b0101);
  printResult("PE", "Load 0101 baseline          ", 0b0101);

  // Now set PE=LOW, apply different P data, set J/K to something
  // that would shift differently if shift mode were active
  digitalWrite(PIN_PE, LOW);
  applyData(0b1010);
  digitalWrite(PIN_J, HIGH);
  digitalWrite(PIN_K, HIGH);
  delay(20);
  pulseClock();
  printResult("PE", "PE=L loads P=1010 not J/K   ", 0b1010);
  digitalWrite(PIN_PE, HIGH);
  delay(10);
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
  digitalWrite(PIN_PE, HIGH);   // shift mode default
  digitalWrite(PIN_J,  LOW);
  digitalWrite(PIN_K,  HIGH);
  for (int i = 0; i < 4; i++) digitalWrite(PIN_P[i], LOW);

  pinMode(PIN_MR, OUTPUT);
  pinMode(PIN_CP, OUTPUT);
  pinMode(PIN_PE, OUTPUT);
  pinMode(PIN_J,  OUTPUT);
  pinMode(PIN_K,  OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_P[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_Q[i], INPUT);
  pinMode(PIN_Q3B, INPUT);

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS195N 4-Bit Shift Register Tester"));
  Serial.println(F("========================================"));

  testAsyncReset();
  testParallelLoad();
  testShiftSetReset();
  testShiftToggle();
  testWalkingOne();
  testPEOverride();

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
