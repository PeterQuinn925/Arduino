// ============================================================
//  74LS85N 4-Bit Magnitude Comparator Tester
//  Arduino Nano
//
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Data inputs — word A:
//   D2  (A0) [10]   Least significant bit
//   D3  (A1) [12]
//   D4  (A2) [13]
//   D5  (A3) [15]   Most significant bit
//
//  Data inputs — word B:
//   D6  (B0) [9]    Least significant bit
//   D7  (B1) [11]
//   D8  (B2) [14]
//   D9  (B3) [1]    Most significant bit
//
//  Cascade inputs (standalone operation — fixed wiring):
//   GND (IA>B) [2]   Tie to GND
//   5V  (IA=B) [3]   Tie to 5V
//   GND (IA<B) [4]   Tie to GND
//
//  Outputs:
//   A0  (OA>B) [5]   HIGH when A > B
//   A1  (OA=B) [6]   HIGH when A = B
//   A2  (OA<B) [7]   HIGH when A < B
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  NOTE: Cascade inputs [2],[3],[4] must be hardwired on the
//        breadboard/clip, not driven by Arduino pins.
//        [2] -> GND, [3] -> 5V, [4] -> GND
// ============================================================

// --- Word A input pins (A0=LSB, A3=MSB) ---
const int xPIN_A0 = 2;//conflict with a already existing macro for PIN_A0. temp change to xPIN_A0 to verify
const int xPIN_A1 = 3;
const int xPIN_A2 = 4;
const int xPIN_A3 = 5;

// --- Word B input pins (B0=LSB, B3=MSB) ---
const int PIN_B0 = 6;
const int PIN_B1 = 7;
const int PIN_B2 = 8;
const int PIN_B3 = 9;

// --- Output pins ---
const int OUT_GT = A0;   // OA>B
const int OUT_EQ = A1;   // OA=B
const int OUT_LT = A2;   // OA<B

// ---------------------------------------------------------------
// Apply a 4-bit value to word A or word B pins
// ---------------------------------------------------------------
void applyWord(int p0, int p1, int p2, int p3, uint8_t val) {
  digitalWrite(p0, (val >> 0) & 1);
  digitalWrite(p1, (val >> 1) & 1);
  digitalWrite(p2, (val >> 2) & 1);
  digitalWrite(p3, (val >> 3) & 1);
}

// ---------------------------------------------------------------
// Read outputs and return as a 3-bit value: bit2=GT, bit1=EQ, bit0=LT
// ---------------------------------------------------------------
uint8_t readOutputs() {
  delay(5);   // propagation settle
  uint8_t gt = digitalRead(OUT_GT);
  uint8_t eq = digitalRead(OUT_EQ);
  uint8_t lt = digitalRead(OUT_LT);
  return (gt << 2) | (eq << 1) | (lt << 0);
}

// ---------------------------------------------------------------
// Print a 4-bit value as a decimal and binary string e.g. " 5 (0101)"
// ---------------------------------------------------------------
void printVal(uint8_t v) {
  if (v < 10) Serial.print(F(" "));
  Serial.print(v);
  Serial.print(F(" ("));
  for (int i = 3; i >= 0; i--) Serial.print((v >> i) & 1);
  Serial.print(F(")"));
}

// ---------------------------------------------------------------
// Print outputs as GT/EQ/LT indicators
// ---------------------------------------------------------------
void printOutputs(uint8_t actual, uint8_t expected) {
  // actual
  Serial.print(F("GT="));  Serial.print((actual >> 2) & 1);
  Serial.print(F(" EQ=")); Serial.print((actual >> 1) & 1);
  Serial.print(F(" LT=")); Serial.print((actual >> 0) & 1);
  Serial.print(F("  exp: "));
  Serial.print(F("GT="));  Serial.print((expected >> 2) & 1);
  Serial.print(F(" EQ=")); Serial.print((expected >> 1) & 1);
  Serial.print(F(" LT=")); Serial.print((expected >> 0) & 1);
}

// ---------------------------------------------------------------
// Main test — runs all 256 A/B combinations
// ---------------------------------------------------------------
void runTest() {
  int passed = 0;
  int failed = 0;

  Serial.println(F("\n  A          B       | GT EQ LT  exp GT EQ LT | result"));
  Serial.println(F("  -----------------------------------------------------------------"));

  for (uint8_t a = 0; a < 16; a++) {
    for (uint8_t b = 0; b < 16; b++) {

      applyWord(xPIN_A0, xPIN_A1, xPIN_A2, xPIN_A3, a);
      applyWord(PIN_B0, PIN_B1, PIN_B2, PIN_B3, b);

      uint8_t actual = readOutputs();

      // Expected: exactly one output HIGH
      uint8_t expected;
      if      (a > b) expected = 0b100;   // GT
      else if (a == b) expected = 0b010;  // EQ
      else             expected = 0b001;  // LT

      bool ok = (actual == expected);
      if (ok) passed++; else failed++;

      // Only print failures and a sample of passes to keep output readable
      // Print all failures, and every 16th pass as a spot-check
      if (!ok || (a == b) || (b == 0 && a == 15) || (a == 0 && b == 15)) {
        Serial.print(F("  "));
        printVal(a);
        Serial.print(F("  "));
        printVal(b);
        Serial.print(F("  | "));
        printOutputs(actual, expected);
        Serial.print(F("  | "));
        Serial.println(ok ? F("PASS") : F("FAIL <<<"));
      }
    }
  }

  Serial.println(F("  -----------------------------------------------------------------"));
  Serial.print(F("  Passed: ")); Serial.print(passed);
  Serial.print(F("  Failed: ")); Serial.println(failed);
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Word A inputs
  pinMode(xPIN_A0, OUTPUT); digitalWrite(xPIN_A0, LOW);
  pinMode(xPIN_A1, OUTPUT); digitalWrite(xPIN_A1, LOW);
  pinMode(xPIN_A2, OUTPUT); digitalWrite(xPIN_A2, LOW);
  pinMode(xPIN_A3, OUTPUT); digitalWrite(xPIN_A3, LOW);

  // Word B inputs
  pinMode(PIN_B0, OUTPUT); digitalWrite(PIN_B0, LOW);
  pinMode(PIN_B1, OUTPUT); digitalWrite(PIN_B1, LOW);
  pinMode(PIN_B2, OUTPUT); digitalWrite(PIN_B2, LOW);
  pinMode(PIN_B3, OUTPUT); digitalWrite(PIN_B3, LOW);

  // Outputs
  pinMode(OUT_GT, INPUT);
  pinMode(OUT_EQ, INPUT);
  pinMode(OUT_LT, INPUT);

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS85N 4-Bit Magnitude Comparator"));
  Serial.println(F("  Tester — 256 A/B combinations"));
  Serial.println(F("========================================"));
  Serial.println(F("  Showing: all failures + spot-check rows"));
  Serial.println(F("  (A=B, A=0 vs B=15, A=15 vs B=0)"));

  runTest();

  Serial.println(F("\n============================="));
  Serial.println(F("Press reset to test again."));
}

// ---------------------------------------------------------------
// loop
// ---------------------------------------------------------------
void loop() {}
