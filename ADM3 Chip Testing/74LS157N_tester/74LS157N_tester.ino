// ============================================================
//  74LS157N Quad 2-Input Data Selector/Multiplexer Tester
//  Arduino Nano
//
//  16-pin DIP package.
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Control inputs (shared across all 4 multiplexers):
//   D2  (S)   [1]   Select: LOW=select A inputs, HIGH=select B inputs
//   D3  (/E)  [15]  Strobe/Enable: active LOW (LOW=enabled, HIGH=disabled)
//
//  Multiplexer 1:
//   D4  (1A)  [2]   Mux 1 input A
//   D5  (1B)  [3]   Mux 1 input B
//   A0  (1Y)  [4]   Mux 1 output
//
//  Multiplexer 2:
//   D6  (2A)  [5]   Mux 2 input A
//   D7  (2B)  [6]   Mux 2 input B
//   A1  (2Y)  [7]   Mux 2 output
//
//  Multiplexer 3:
//   D8  (3A)  [11]  Mux 3 input A
//   D9  (3B)  [10]  Mux 3 input B
//   A2  (3Y)  [9]   Mux 3 output
//
//  Multiplexer 4:
//   D10 (4A)  [14]  Mux 4 input A
//   D11 (4B)  [13]  Mux 4 input B
//   A3  (4Y)  [12]  Mux 4 output
//
//   VCC [16] -> 5V
//   GND [8]  -> GND
//
//  Truth table (all 4 muxes behave identically):
//   /E   S  | Y
//   H    X  | L    (disabled — output always LOW)
//   L    L  | A    (select A input)
//   L    H  | B    (select B input)
//
//  NOTE: Outputs are non-inverting (true, not complemented).
// ============================================================

// --- Control pins ---
const int PIN_S  = 2;    // select
const int PIN_E  = 3;    // strobe /E (active LOW)

// --- A inputs (source 0) ---
const int PIN_A[4] = {4, 6, 8, 10};    // 1A, 2A, 3A, 4A

// --- B inputs (source 1) ---
const int PIN_B[4] = {5, 7, 9, 11};    // 1B, 2B, 3B, 4B

// --- Outputs ---
const int PIN_Y[4] = {A0, A1, A2, A3}; // 1Y, 2Y, 3Y, 4Y

int totalPass = 0;
int totalFail = 0;

// ---------------------------------------------------------------
// Print separator
// ---------------------------------------------------------------
void printSep() {
  Serial.println(F("  +----+---+----+----+----++----+----+----+----++--------+"));
}

// ---------------------------------------------------------------
// Print table header
// ---------------------------------------------------------------
void printHeader() {
  printSep();
  Serial.println(F("  | /E | S | 1A | 1B | 1Y || 2A | 2B | 2Y || ...     |"));
  printSep();
}

// ---------------------------------------------------------------
// Check and print one full row (all 4 muxes simultaneously)
// enabled: true when /E=LOW
// sel: false=A inputs selected, true=B inputs selected
// aVals/bVals: the 4-bit patterns applied to A and B inputs
// ---------------------------------------------------------------
void checkRow(bool enabled, bool selB,
              uint8_t aVals, uint8_t bVals) {
  delay(5);

  bool rowOk = true;

  // Read all 4 outputs
  int y[4];
  for (int m = 0; m < 4; m++) {
    y[m] = digitalRead(PIN_Y[m]);
  }

  // Expected: disabled->0, selB->B input, selA->A input
  int exp[4];
  for (int m = 0; m < 4; m++) {
    if (!enabled) {
      exp[m] = 0;
    } else if (selB) {
      exp[m] = (bVals >> m) & 1;
    } else {
      exp[m] = (aVals >> m) & 1;
    }
    if (y[m] != exp[m]) rowOk = false;
  }

  if (rowOk) totalPass++; else totalFail++;

  // Print row
  // | /E | S | 1A 1B 1Y | 2A 2B 2Y | 3A 3B 3Y | 4A 4B 4Y | result |
  Serial.print(F("  |  "));
  Serial.print(enabled ? F("L") : F("H"));
  Serial.print(F(" | "));
  Serial.print(!enabled ? F("X") : (selB ? F("H") : F("L")));
  Serial.print(F(" |"));
  for (int m = 0; m < 4; m++) {
    int a = (aVals >> m) & 1;
    int b = (bVals >> m) & 1;
    Serial.print(F("  "));
    Serial.print(!enabled ? F("X") : String(a));
    Serial.print(F("   "));
    Serial.print(!enabled ? F("X") : String(b));
    Serial.print(F("   "));
    Serial.print(y[m]);
    Serial.print(F(" |"));
    if (m == 1) Serial.print(F("|"));
  }
  Serial.print(F("  "));
  // Expected summary
  Serial.print(F("exp:"));
  for (int m = 0; m < 4; m++) {
    Serial.print(exp[m]);
  }
  Serial.print(F("  "));
  Serial.println(rowOk ? F("PASS") : F("FAIL <<<"));
}

// ---------------------------------------------------------------
// Phase 1: Disabled test (/E=HIGH)
// All outputs must be LOW regardless of S, A, B
// ---------------------------------------------------------------
void testDisabled() {
  Serial.println(F("\n--- Phase 1: Strobe /E=HIGH (disabled) ---"));
  Serial.println(F("  All Y outputs must be LOW regardless of S, A, B inputs"));
  Serial.println(F("\n  /E   S   1A  1B  1Y   2A  2B  2Y  | 3A  3B  3Y   4A  4B  4Y  |  exp  result"));
  Serial.println(F("  ---------------------------------------------------------------------------------"));

  digitalWrite(PIN_E, HIGH);   // disable

  // Walk through S=0 and S=1, and all 4 A/B combinations per mux
  for (int sel = 0; sel < 2; sel++) {
    digitalWrite(PIN_S, sel);
    for (uint8_t ab = 0; ab < 16; ab++) {
      // Apply same pattern to both A and B to maximise stress
      for (int m = 0; m < 4; m++) {
        digitalWrite(PIN_A[m], (ab >> m) & 1);
        digitalWrite(PIN_B[m], (ab >> m) & 1);
      }
      delay(5);
      checkRow(false, sel, ab, ab);
    }
  }
}

// ---------------------------------------------------------------
// Phase 2: Select A (/E=LOW, S=LOW)
// Each Y output must follow its A input
// ---------------------------------------------------------------
void testSelectA() {
  Serial.println(F("\n--- Phase 2: /E=LOW S=LOW (select A inputs) ---"));
  Serial.println(F("  Each Y must follow its own A input; B inputs are irrelevant"));
  Serial.println(F("\n  /E   S   1A  1B  1Y   2A  2B  2Y  | 3A  3B  3Y   4A  4B  4Y  |  exp  result"));
  Serial.println(F("  ---------------------------------------------------------------------------------"));

  digitalWrite(PIN_E, LOW);
  digitalWrite(PIN_S, LOW);

  // Walk A inputs through all 16 combinations; B inputs fixed LOW
  for (uint8_t aVals = 0; aVals < 16; aVals++) {
    for (int m = 0; m < 4; m++) {
      digitalWrite(PIN_A[m], (aVals >> m) & 1);
      digitalWrite(PIN_B[m], LOW);
    }
    delay(5);
    checkRow(true, false, aVals, 0x00);
  }

  // Now walk B inputs — Y should NOT follow (B is not selected)
  Serial.println(F("  -- B inputs walking (Y must stay matching A=0) --"));
  for (int m = 0; m < 4; m++) digitalWrite(PIN_A[m], LOW);
  for (uint8_t bVals = 1; bVals < 16; bVals++) {
    for (int m = 0; m < 4; m++) {
      digitalWrite(PIN_B[m], (bVals >> m) & 1);
    }
    delay(5);
    checkRow(true, false, 0x00, bVals);
  }
}

// ---------------------------------------------------------------
// Phase 3: Select B (/E=LOW, S=HIGH)
// Each Y output must follow its B input
// ---------------------------------------------------------------
void testSelectB() {
  Serial.println(F("\n--- Phase 3: /E=LOW S=HIGH (select B inputs) ---"));
  Serial.println(F("  Each Y must follow its own B input; A inputs are irrelevant"));
  Serial.println(F("\n  /E   S   1A  1B  1Y   2A  2B  2Y  | 3A  3B  3Y   4A  4B  4Y  |  exp  result"));
  Serial.println(F("  ---------------------------------------------------------------------------------"));

  digitalWrite(PIN_E, LOW);
  digitalWrite(PIN_S, HIGH);

  // Walk B inputs through all 16 combinations; A inputs fixed LOW
  for (uint8_t bVals = 0; bVals < 16; bVals++) {
    for (int m = 0; m < 4; m++) {
      digitalWrite(PIN_B[m], (bVals >> m) & 1);
      digitalWrite(PIN_A[m], LOW);
    }
    delay(5);
    checkRow(true, true, 0x00, bVals);
  }

  // Now walk A inputs — Y should NOT follow (A is not selected)
  Serial.println(F("  -- A inputs walking (Y must stay matching B=0) --"));
  for (int m = 0; m < 4; m++) digitalWrite(PIN_B[m], LOW);
  for (uint8_t aVals = 1; aVals < 16; aVals++) {
    for (int m = 0; m < 4; m++) {
      digitalWrite(PIN_A[m], (aVals >> m) & 1);
    }
    delay(5);
    checkRow(true, true, aVals, 0x00);
  }
}

// ---------------------------------------------------------------
// Phase 4: Select switching test
// Verify outputs switch cleanly when S toggles
// ---------------------------------------------------------------
void testSelectSwitch() {
  Serial.println(F("\n--- Phase 4: Select switching (S toggles A<->B) ---"));
  Serial.println(F("  A and B inputs are opposite; Y must follow S"));
  Serial.println(F("\n  /E   S   1A  1B  1Y   2A  2B  2Y  | 3A  3B  3Y   4A  4B  4Y  |  exp  result"));
  Serial.println(F("  ---------------------------------------------------------------------------------"));

  digitalWrite(PIN_E, LOW);

  // Set A=1010, B=0101 (alternating opposites per mux)
  for (int m = 0; m < 4; m++) {
    digitalWrite(PIN_A[m], (m % 2 == 0) ? HIGH : LOW);   // 1A=1, 2A=0, 3A=1, 4A=0
    digitalWrite(PIN_B[m], (m % 2 == 0) ? LOW  : HIGH);  // 1B=0, 2B=1, 3B=0, 4B=1
  }
  uint8_t aPattern = 0b0101;  // bits: mux0=1,mux1=0,mux2=1,mux3=0
  uint8_t bPattern = 0b1010;

  for (int t = 0; t < 8; t++) {
    bool selB = (t % 2 != 0);
    digitalWrite(PIN_S, selB ? HIGH : LOW);
    delay(10);
    checkRow(true, selB, aPattern, bPattern);
  }
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Control pins
  digitalWrite(PIN_S, LOW);   pinMode(PIN_S, OUTPUT);
  digitalWrite(PIN_E, HIGH);  pinMode(PIN_E, OUTPUT);  // start disabled

  // A and B input pins
  for (int m = 0; m < 4; m++) {
    digitalWrite(PIN_A[m], LOW); pinMode(PIN_A[m], OUTPUT);
    digitalWrite(PIN_B[m], LOW); pinMode(PIN_B[m], OUTPUT);
  }

  // Output pins
  for (int m = 0; m < 4; m++) {
    pinMode(PIN_Y[m], INPUT);
  }

  delay(100);

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS157N Quad 2-Input Multiplexer"));
  Serial.println(F("  Tester"));
  Serial.println(F("========================================"));

  testDisabled();
  testSelectA();
  testSelectB();
  testSelectSwitch();

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
