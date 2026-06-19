// ============================================================
//  74LS283N 4-Bit Binary Full Adder With Fast Carry Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  A Inputs:
//   D2  (A1) [5]    A bit 0 (LSB)
//   D3  (A2) [3]    A bit 1
//   D4  (A3) [14]   A bit 2
//   D5  (A4) [12]   A bit 3 (MSB)
//
//  B Inputs:
//   D6  (B1) [6]    B bit 0 (LSB)
//   D7  (B2) [2]    B bit 1
//   D8  (B3) [15]   B bit 2
//   D9  (B4) [11]   B bit 3 (MSB)
//
//  Carry Input:
//   D10 (C0) [7]    Carry In
//
//  Sum Outputs:
//   A0  (S1) [4]    Sum bit 0 (LSB)
//   A1  (S2) [1]    Sum bit 1
//   A2  (S3) [13]   Sum bit 2
//   A3  (S4) [10]   Sum bit 3 (MSB)
//
//  Carry Output:
//   A4  (C4) [9]    Carry Out
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  How it works:
//   The 74LS283 performs:
//
//       SUM = A + B + C0
//
//   where:
//       A  = 4-bit number (0-15)
//       B  = 4-bit number (0-15)
//       C0 = carry input (0 or 1)
//
//   Outputs:
//       S4 S3 S2 S1 = lower 4 bits of result
//       C4          = carry out (5th bit)
//
//   Example:
//       A  = 1010 (10)
//       B  = 0111 (7)
//       C0 = 1
//
//       Result = 18 (10010)
//
//       S4..S1 = 0010
//       C4     = 1
//
// ============================================================

// --- A inputs ---
const int A_PINS[4] = {2, 3, 4, 5};     // A1..A4

// --- B inputs ---
const int B_PINS[4] = {6, 7, 8, 9};     // B1..B4

// --- Carry input ---
const int PIN_C0 = 10;

// --- Sum outputs ---
const int S_PINS[4] = {A0, A1, A2, A3}; // S1..S4

// --- Carry output ---
const int PIN_C4 = A4;

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Apply 4-bit value to input pins
// ---------------------------------------------------------------
void setNibble(const int pins[4], uint8_t value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(pins[i], (value >> i) & 1);
  }
}

// ---------------------------------------------------------------
// Read 4-bit sum output
// ---------------------------------------------------------------
uint8_t readSum() {
  uint8_t value = 0;

  for (int i = 0; i < 4; i++) {
    if (digitalRead(S_PINS[i])) {
      value |= (1 << i);
    }
  }

  return value;
}

// ---------------------------------------------------------------
// Print separator
// ---------------------------------------------------------------
void printSeparator() {
  Serial.println(F("-------------------------------------------------------------------------------------"));
}

// ---------------------------------------------------------------
// Print binary nibble
// ---------------------------------------------------------------
void printNibble(uint8_t value) {
  for (int i = 3; i >= 0; i--) {
    Serial.print((value >> i) & 1);
  }
}

// ---------------------------------------------------------------
// Test one combination
// ---------------------------------------------------------------
bool checkAndPrint(uint8_t A, uint8_t B, uint8_t Cin) {

  setNibble(A_PINS, A);
  setNibble(B_PINS, B);

  digitalWrite(PIN_C0, Cin);

  delay(5);

  uint8_t actualSum = readSum();
  uint8_t actualCarry = digitalRead(PIN_C4);

  uint16_t expected = A + B + Cin;

  uint8_t expectedSum = expected & 0x0F;
  uint8_t expectedCarry = (expected >> 4) & 0x01;

  bool pass =
    (actualSum == expectedSum) &&
    (actualCarry == expectedCarry);

  if (pass)
    totalPass++;
  else
    totalFail++;

  // Row format
  Serial.print(F("A="));
  printNibble(A);

  Serial.print(F(" ("));
  Serial.print(A);
  Serial.print(F(")  "));

  Serial.print(F("B="));
  printNibble(B);

  Serial.print(F(" ("));
  Serial.print(B);
  Serial.print(F(")  "));

  Serial.print(F("Cin="));
  Serial.print(Cin);

  Serial.print(F("  |  Actual="));

  Serial.print(actualCarry);
  printNibble(actualSum);

  Serial.print(F(" ("));
  Serial.print((actualCarry << 4) | actualSum);
  Serial.print(F(")"));

  Serial.print(F("  |  Expected="));

  Serial.print(expectedCarry);
  printNibble(expectedSum);

  Serial.print(F(" ("));
  Serial.print(expected);
  Serial.print(F(")"));

  Serial.print(F("  |  "));

  Serial.println(pass ? F("PASS") : F("FAIL <<<"));

  return pass;
}

// ---------------------------------------------------------------
// Full exhaustive test
// 16 x 16 x 2 = 512 combinations
// ---------------------------------------------------------------
void exhaustiveTest() {

  Serial.println(F("\n--- Exhaustive Truth Table Test ---"));

  printSeparator();

  Serial.println(
    F("A (dec/bin)      B (dec/bin)      Cin | Actual(C4S4S3S2S1) | Expected | Result"));

  printSeparator();

  for (uint8_t cin = 0; cin <= 1; cin++) {

    Serial.println();
    Serial.print(F("Carry In = "));
    Serial.println(cin);

    printSeparator();

    for (uint8_t a = 0; a < 16; a++) {
      for (uint8_t b = 0; b < 16; b++) {
        checkAndPrint(a, b, cin);
      }
    }
  }

  printSeparator();
}

// ---------------------------------------------------------------
// Walking bit test
// Quickly checks every input line independently
// ---------------------------------------------------------------
void walkingBitTest() {

  Serial.println(F("\n--- Walking Bit Test ---"));

  printSeparator();

  for (uint8_t bit = 0; bit < 4; bit++) {

    uint8_t val = (1 << bit);

    checkAndPrint(val, 0, 0);
    checkAndPrint(0, val, 0);
    checkAndPrint(val, val, 0);
  }

  printSeparator();
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {

  Serial.begin(9600);
  while (!Serial) {}

  // A inputs
  for (int i = 0; i < 4; i++) {
    pinMode(A_PINS[i], OUTPUT);
    digitalWrite(A_PINS[i], LOW);
  }

  // B inputs
  for (int i = 0; i < 4; i++) {
    pinMode(B_PINS[i], OUTPUT);
    digitalWrite(B_PINS[i], LOW);
  }

  // Carry input
  pinMode(PIN_C0, OUTPUT);
  digitalWrite(PIN_C0, LOW);

  // Outputs
  for (int i = 0; i < 4; i++) {
    pinMode(S_PINS[i], INPUT);
  }

  pinMode(PIN_C4, INPUT);

  delay(100);

  Serial.println();
  Serial.println(F("===================================================="));
  Serial.println(F("     74LS283N 4-Bit Binary Full Adder Tester"));
  Serial.println(F("===================================================="));
  Serial.println(F("Tests all 512 possible input combinations"));
  Serial.println(F("Result shown as C4S4S3S2S1"));
  Serial.println(F("===================================================="));

  walkingBitTest();
  exhaustiveTest();

  Serial.println();
  Serial.println(F("================ SUMMARY ================"));

  Serial.print(F("Passed: "));
  Serial.println(totalPass);

  Serial.print(F("Failed: "));
  Serial.println(totalFail);

  Serial.println(F("-----------------------------------------"));

  if (totalFail == 0)
    Serial.println(F("Chip Status: GOOD"));
  else
    Serial.println(F("Chip Status: FAULTY"));

  Serial.println(F("========================================="));
  Serial.println(F("\nPress RESET to run again."));
}

// ---------------------------------------------------------------
// loop
// ---------------------------------------------------------------
void loop() {
}