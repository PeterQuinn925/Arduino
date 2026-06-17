// ============================================================
//  74LS10N Triple 3-Input NAND Gate Tester
//  Arduino Nano
//
//  Wiring:
//   Gate 1 inputs : D2(1A)[1], D3(1B)[2], D4(1C)[13]   output: A0(1Y)[12]
//   Gate 2 inputs : D5(2A)[3], D6(2B)[4], D7(2C)[5]   output: A1(2Y)[6]
//   Gate 3 inputs : D8(3A)[9], D9(3B)[10], D10(3C)[11]  output: A2(3Y)[8]
//   VCC  -> 5V[14]  |  GND -> GND[7]
//
//  The NOR truth table for each gate:
//   A B C | Y
//   0 0 0 | 1   <-- only HIGH when ALL inputs LOW
//   0 0 1 | 0
//   0 1 0 | 0
//   0 1 1 | 0
//   1 0 0 | 0
//   1 0 1 | 0
//   1 1 0 | 0
//   1 1 1 | 0
//
//  Results are printed over Serial (115200 baud).
// ============================================================

// --- Pin definitions ---
const int GATE1_INPUTS[3] = {2, 3,  4};   // A1, B1, C1
const int GATE2_INPUTS[3] = {5, 6,  7};   // A2, B2, C2
const int GATE3_INPUTS[3] = {8, 9, 10};   // A3, B3, C3

const int GATE1_OUTPUT = A0;  // Y1
const int GATE2_OUTPUT = A1;  // Y2
const int GATE3_OUTPUT = A2;  // Y3

// Collect results
bool gatePass[3];
int  failCombo[3];   // first failing combo index (-1 = none)

// ---------------------------------------------------------------
// Apply a 3-bit combo (0..7) to three output pins
// ---------------------------------------------------------------
void applyInputs(const int pins[3], uint8_t combo) {
  digitalWrite(pins[0], (combo >> 2) & 1);
  digitalWrite(pins[1], (combo >> 1) & 1);
  digitalWrite(pins[2], (combo >> 0) & 1);
  delayMicroseconds(10);   // propagation delay headroom (~50 ns typical)
}

// ---------------------------------------------------------------
// Expected NOR output: HIGH only when all three inputs are LOW
// ---------------------------------------------------------------
bool expectedNOR(uint8_t combo) {
  return (combo == 0);
}
bool expectedNAND(uint8_t combo) {
  return (combo != 3);  // LOW only when both inputs HIGH
}
bool expectedNAND_3input(uint8_t combo){
  return (combo != 7);
}

bool expectedAND(uint8_t combo) {
return (combo == 3);
}

// ---------------------------------------------------------------
// Test one gate; return true if all 8 combos pass
// ---------------------------------------------------------------
bool testGate(int gateNum,
              const int inputPins[3],
              int outputPin) {

  Serial.print(F("\n--- Gate "));
  Serial.print(gateNum);
  Serial.println(F(" ---"));
  Serial.println(F(" A  B  C | Y_actual | Y_expect | result"));

  bool allOk = true;
  int firstFail = -1;

  for (uint8_t combo = 0; combo < 8; combo++) {
    applyInputs(inputPins, combo);
    delay(200);
    int a       = (combo >> 2) & 1;
    int b       = (combo >> 1) & 1;
    int c       = (combo >> 0) & 1;
    int actual  = digitalRead(outputPin);
    int expected = expectedNAND_3input(combo) ? 1 : 0;
    bool ok      = (actual == expected);

    if (!ok && allOk) {          // record first failure
      allOk = false;
      firstFail = combo;
    }

    // Print row
    Serial.print(F(" "));
    Serial.print(a); Serial.print(F("  "));
    Serial.print(b); Serial.print(F("  "));
    Serial.print(c); Serial.print(F(" |    "));
    Serial.print(actual);
    Serial.print(F("     |    "));
    Serial.print(expected);
    Serial.print(F("     | "));
    Serial.println(ok ? F("PASS") : F("FAIL <<<"));
  }

  // Drive all inputs LOW after test
  applyInputs(inputPins, 0);

  failCombo[gateNum - 1] = firstFail;
  return allOk;
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}            // wait for USB serial on Nano

  // Configure input driver pins as OUTPUTs, default LOW
  for (int i = 0; i < 3; i++) {
    pinMode(GATE1_INPUTS[i], OUTPUT); digitalWrite(GATE1_INPUTS[i], LOW);
    pinMode(GATE2_INPUTS[i], OUTPUT); digitalWrite(GATE2_INPUTS[i], LOW);
    pinMode(GATE3_INPUTS[i], OUTPUT); digitalWrite(GATE3_INPUTS[i], LOW);
  }

  // Configure output sense pins as INPUTs
  pinMode(GATE1_OUTPUT, INPUT_PULLUP);
  pinMode(GATE2_OUTPUT, INPUT_PULLUP);
  pinMode(GATE3_OUTPUT, INPUT_PULLUP);

  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS10 Triple NAND Gate Tester"));
  Serial.println(F("========================================"));

  // Run tests
  gatePass[0] = testGate(1, GATE1_INPUTS, GATE1_OUTPUT);
  gatePass[1] = testGate(2, GATE2_INPUTS, GATE2_OUTPUT);
  gatePass[2] = testGate(3, GATE3_INPUTS, GATE3_OUTPUT);

  // Summary
  Serial.println(F("\n========== SUMMARY =========="));
  bool chipOk = true;
  for (int g = 0; g < 3; g++) {
    Serial.print(F("Gate "));
    Serial.print(g + 1);
    Serial.print(F(": "));
    if (gatePass[g]) {
      Serial.println(F("PASS"));
    } else {
      Serial.print(F("FAIL  (first fail on combo "));
      Serial.print(failCombo[g], BIN);
      Serial.println(F(")"));
      chipOk = false;
    }
  }
  Serial.println(F("-----------------------------"));
  Serial.println(chipOk ? F("Chip: GOOD") : F("Chip: FAULTY"));
  Serial.println(F("============================="));
  Serial.println(F("\nPress reset to test again."));
}

// ---------------------------------------------------------------
// loop — nothing to do, all work done in setup
// ---------------------------------------------------------------
void loop() {}
