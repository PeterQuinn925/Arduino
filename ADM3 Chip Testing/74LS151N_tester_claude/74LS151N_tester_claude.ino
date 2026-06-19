// ============================================================
//  74LS151N 8-Input Data Selector/Multiplexer Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Data inputs:
//   D2  (I0) [4]    Data input 0
//   D3  (I1) [3]    Data input 1
//   D4  (I2) [2]    Data input 2
//   D5  (I3) [1]    Data input 3
//   D6  (I4) [15]   Data input 4
//   D7  (I5) [14]   Data input 5
//   D8  (I6) [13]   Data input 6
//   D9  (I7) [12]   Data input 7
//
//  Select inputs:
//   D10 (S0) [9]    Select bit 0 (LSB)
//   D11 (S1) [10]   Select bit 1
//   D12 (S2) [11]   Select bit 2 (MSB)
//
//  Enable:
//   D13 (E)  [7]    Active LOW enable (LOW = enabled)
//
//  Outputs:
//   A0  (Z)  [5]    True output  (follows selected input)
//   A1  (/Z) [6]    Complement output (inverts selected input)
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  How it works:
//   S2,S1,S0 select one of the 8 data inputs (I0-I7).
//   Z output = selected input value (when E=LOW).
//   /Z output = inverted selected input value.
//   When E=HIGH (disabled): Z=LOW, /Z=HIGH regardless of inputs.
//
//  Truth table:
//   E  S2 S1 S0 | Z    /Z
//   H  X  X  X  | L    H     (disabled)
//   L  0  0  0  | I0   /I0
//   L  0  0  1  | I1   /I1
//   L  0  1  0  | I2   /I2
//   L  0  1  1  | I3   /I3
//   L  1  0  0  | I4   /I4
//   L  1  0  1  | I5   /I5
//   L  1  1  0  | I6   /I6
//   L  1  1  1  | I7   /I7
// ============================================================

// --- Data input pins (I0=LSB address 0 .. I7=address 7) ---
const int DATA_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 9};  // I0..I7

// --- Select input pins ---
const int PIN_S0 = 10;   // LSB
const int PIN_S1 = 11;
const int PIN_S2 = 12;   // MSB

// --- Enable pin (active LOW) ---
const int PIN_E  = 13;

// --- Output pins ---
const int OUT_Z  = A0;   // true output
const int OUT_ZB = A1;   // complement output

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Apply select address (0-7) to S0, S1, S2
// ---------------------------------------------------------------
void applySelect(uint8_t sel) {
  digitalWrite(PIN_S0, (sel >> 0) & 1);
  digitalWrite(PIN_S1, (sel >> 1) & 1);
  digitalWrite(PIN_S2, (sel >> 2) & 1);
}

// ---------------------------------------------------------------
// Print a separator line
// ---------------------------------------------------------------
void printSeparator() {
  Serial.println(F("  -----------------------------------------------------------------------"));
}

// ---------------------------------------------------------------
// Check one combination and print a result row
// Returns true if pass
// ---------------------------------------------------------------
bool checkAndPrint(uint8_t sel, uint8_t dataVal, bool enabled) {
  delay(5);
  int actualZ  = digitalRead(OUT_Z);
  int actualZB = digitalRead(OUT_ZB);

  int expectedZ, expectedZB;
  if (!enabled) {
    expectedZ  = 0;   // disabled: Z always LOW
    expectedZB = 1;   // /Z always HIGH
  } else {
    expectedZ  = dataVal;
    expectedZB = !dataVal;
  }

  bool ok = (actualZ == expectedZ) && (actualZB == expectedZB);
  if (ok) totalPass++; else totalFail++;

  // Row format:
  //  E  S2 S1 S0 | I_sel | Z_act /Z_act | Z_exp /Z_exp | result
  Serial.print(F("  "));
  Serial.print(enabled ? F("L") : F("H"));
  Serial.print(F("   "));
  Serial.print((sel >> 2) & 1);
  Serial.print(F("  "));
  Serial.print((sel >> 1) & 1);
  Serial.print(F("  "));
  Serial.print((sel >> 0) & 1);
  Serial.print(F("   I"));
  Serial.print(sel);
  Serial.print(F("="));
  Serial.print(dataVal);
  if (!enabled) Serial.print(F(" (X)"));
  else          Serial.print(F("    "));
  Serial.print(F("  |  Z="));
  Serial.print(actualZ);
  Serial.print(F(" /Z="));
  Serial.print(actualZB);
  Serial.print(F("  |  Z="));
  Serial.print(expectedZ);
  Serial.print(F(" /Z="));
  Serial.print(expectedZB);
  Serial.print(F("  |  "));
  Serial.println(ok ? F("PASS") : F("FAIL <<<"));

  return ok;
}

// ---------------------------------------------------------------
// Phase 1: Disabled test (E=HIGH)
// All select/data combinations should give Z=0, /Z=1
// ---------------------------------------------------------------
void testDisabled() {
  Serial.println(F("\n--- Phase 1: Enable=HIGH (disabled) ---"));
  Serial.println(F("  All outputs should be Z=0 /Z=1 regardless of inputs"));
  printSeparator();
  Serial.println(F("  E   S2 S1 S0  Input    |  Z /Z actual  |  Z /Z expect  |  result"));
  printSeparator();

  digitalWrite(PIN_E, HIGH);   // disable
  // Try a sample of select and data combinations
  for (uint8_t sel = 0; sel < 8; sel++) {
    applySelect(sel);
    // Set selected input HIGH then LOW — output should stay 0 either way
    for (uint8_t dval = 0; dval < 2; dval++) {
      // Set all inputs to dval
      for (int i = 0; i < 8; i++) digitalWrite(DATA_PINS[i], dval);
      delay(5);
      checkAndPrint(sel, dval, false);
    }
  }
  printSeparator();
}

// ---------------------------------------------------------------
// Phase 2: Enabled test (E=LOW)
// For each of the 8 select values, set the selected input HIGH
// and all others LOW, then LOW and all others HIGH.
// Z should track the selected input exactly.
// ---------------------------------------------------------------
void testEnabled() {
  Serial.println(F("\n--- Phase 2: Enable=LOW (enabled) ---"));
  Serial.println(F("  Z should match selected input Isel; /Z should be complement"));
  printSeparator();
  Serial.println(F("  E   S2 S1 S0  Input    |  Z /Z actual  |  Z /Z expect  |  result"));
  printSeparator();

  digitalWrite(PIN_E, LOW);   // enable

  for (uint8_t sel = 0; sel < 8; sel++) {
    applySelect(sel);
    delay(10);

    // Test selected input = LOW (all inputs LOW)
    for (int i = 0; i < 8; i++) digitalWrite(DATA_PINS[i], LOW);
    delay(10);
    checkAndPrint(sel, 0, true);

    // Test selected input = HIGH (only selected input HIGH)
    for (int i = 0; i < 8; i++) digitalWrite(DATA_PINS[i], LOW);
    digitalWrite(DATA_PINS[sel], HIGH);
    delay(10);
    checkAndPrint(sel, 1, true);

    // Test with all inputs HIGH (selected should still read HIGH)
    for (int i = 0; i < 8; i++) digitalWrite(DATA_PINS[i], HIGH);
    delay(10);
    checkAndPrint(sel, 1, true);

    // Test selected input = LOW with all others HIGH
    for (int i = 0; i < 8; i++) digitalWrite(DATA_PINS[i], HIGH);
    digitalWrite(DATA_PINS[sel], LOW);
    delay(10);
    checkAndPrint(sel, 0, true);

    printSeparator();
  }
}

// ---------------------------------------------------------------
// Phase 3: Walking 1 test
// One input HIGH at a time; select each address in turn.
// Verifies that each data line is correctly routed to the output
// and that no other data line bleeds through.
// ---------------------------------------------------------------
void testWalkingOne() {
  Serial.println(F("\n--- Phase 3: Walking 1 (crosstalk check) ---"));
  Serial.println(F("  One input HIGH; select steps through all 8 addresses"));
  Serial.println(F("  Only the matching address should give Z=1"));
  printSeparator();
  Serial.println(F("  E   S2 S1 S0  Input    |  Z /Z actual  |  Z /Z expect  |  result"));
  printSeparator();

  digitalWrite(PIN_E, LOW);

  for (uint8_t hotPin = 0; hotPin < 8; hotPin++) {
    // Set only hotPin HIGH
    for (int i = 0; i < 8; i++) digitalWrite(DATA_PINS[i], LOW);
    digitalWrite(DATA_PINS[hotPin], HIGH);
    delay(10);

    for (uint8_t sel = 0; sel < 8; sel++) {
      applySelect(sel);
      delay(10);
      uint8_t expectedData = (sel == hotPin) ? 1 : 0;
      checkAndPrint(sel, expectedData, true);
    }
    printSeparator();
  }
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Data inputs
  for (int i = 0; i < 8; i++) {
    digitalWrite(DATA_PINS[i], LOW);
    pinMode(DATA_PINS[i], OUTPUT);
  }

  // Select inputs
  digitalWrite(PIN_S0, LOW);  pinMode(PIN_S0, OUTPUT);
  digitalWrite(PIN_S1, LOW);  pinMode(PIN_S1, OUTPUT);
  digitalWrite(PIN_S2, LOW);  pinMode(PIN_S2, OUTPUT);

  // Enable — start disabled
  digitalWrite(PIN_E, HIGH);  pinMode(PIN_E, OUTPUT);

  // Outputs
  pinMode(OUT_Z,  INPUT);
  pinMode(OUT_ZB, INPUT);

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS151N 8-Input Multiplexer Tester"));
  Serial.println(F("========================================"));

  testDisabled();
  testEnabled();
  testWalkingOne();

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
