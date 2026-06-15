// ============================================================
//  74LS00N 4x 2-Input Postive-NAND Gate Tester
//  Arduino Nano
//
//  NEW Wiring: Arduino Pin (signal name) [chip pin number]
//   Gate 1 inputs : D3(1A)[1], D5(1B)[2]   output: D2(1Y)[3]
//   Gate 2 inputs : D7(2A)[4], A1(2B)[5]   output: D6(2Y)[6]
//   Gate 3 inputs : A2(3A)[10], D8(3B)[9],  output: D9(3Y)[8]
//   Gate 4 inputs : D10(4A)[13], A0(4B)[12],  output: D4(4Y)[11]
//   VCC[14]  -> 5V  |  GND[7] -> GND
//
//
//  Results are printed over Serial 
// ============================================================

// --- Pin definitions ---
const int GATE1_INPUTS[2] = {3,5};   
const int GATE2_INPUTS[2] = {7,A1};   
const int GATE3_INPUTS[2] = {A2,8};
const int GATE4_INPUTS[2] = {10,A0};

const int GATE1_OUTPUT = 2;  // Y1
const int GATE2_OUTPUT = 6;  // Y2
const int GATE3_OUTPUT = 9;  // Y3
const int GATE4_OUTPUT = 4;  // Y3

// Collect results
bool gatePass[4];
int  failCombo[4];   // first failing combo index (-1 = none)

// ---------------------------------------------------------------
// Apply a 3-bit combo (0..7) to three output pins
// ---------------------------------------------------------------
void applyInputs(const int pins[2], uint8_t combo) {
  digitalWrite(pins[0], (combo >> 1) & 1);
  digitalWrite(pins[1], (combo >> 0) & 1);
  delayMicroseconds(10);   // propagation delay headroom (~50 ns typical)
}

// ---------------------------------------------------------------
// Expected AND output: L when both H, otherwise H
// ---------------------------------------------------------------
bool expectedNOR(uint8_t combo) {
  return (combo == 0);
}
bool expectedNAND(uint8_t combo) {
  return (combo != 3);  // LOW only when both inputs HIGH
}
// ---------------------------------------------------------------
// Test one gate; return true if all 8 combos pass
// ---------------------------------------------------------------
bool testGate(int gateNum,
              const int inputPins[2],
              int outputPin) {

  Serial.print(F("\n--- Gate "));
  Serial.print(gateNum);
  Serial.println(F(" ---"));
  Serial.println(F(" A  B  | Y_actual | Y_expect | result"));

  bool allOk = true;
  int firstFail = -1;

  for (uint8_t combo = 0; combo < 4; combo++) {
    applyInputs(inputPins, combo);
    delay(200);
    int a       = (combo >> 1) & 1;
    int b       = (combo >> 0) & 1;
    int actual  = digitalRead(outputPin);
    int expected = expectedNAND(combo) ? 1 : 0;
    bool ok      = (actual == expected);

    if (!ok && allOk) {          // record first failure
      allOk = false;
      firstFail = combo;
    }

    // Print row
    Serial.print(F(" "));
    Serial.print(a); Serial.print(F("  "));
    Serial.print(b); Serial.print(F("  "));
    Serial.print(F(" |    "));
    Serial.print(actual);
    Serial.print(F("     |    "));
    Serial.print(expected);
    Serial.print(F("     | "));
    Serial.println(ok ? F("PASS") : F("FAIL <<<"));
    delay(200);
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
  for (int i = 0; i < 2; i++) {
    pinMode(GATE1_INPUTS[i], OUTPUT); digitalWrite(GATE1_INPUTS[i], LOW);
    pinMode(GATE2_INPUTS[i], OUTPUT); digitalWrite(GATE2_INPUTS[i], LOW);
    pinMode(GATE3_INPUTS[i], OUTPUT); digitalWrite(GATE3_INPUTS[i], LOW);
    pinMode(GATE4_INPUTS[i], OUTPUT); digitalWrite(GATE4_INPUTS[i], LOW);
  }

  // Configure output sense pins as INPUTs
  pinMode(GATE1_OUTPUT, INPUT_PULLUP);
  pinMode(GATE2_OUTPUT, INPUT_PULLUP);
  pinMode(GATE3_OUTPUT, INPUT_PULLUP);
  pinMode(GATE4_OUTPUT, INPUT_PULLUP);

  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS00N 4x NAND Gate Tester"));
  Serial.println(F("========================================"));

  // Run tests
  gatePass[0] = testGate(1, GATE1_INPUTS, GATE1_OUTPUT);
  gatePass[1] = testGate(2, GATE2_INPUTS, GATE2_OUTPUT);
  gatePass[2] = testGate(3, GATE3_INPUTS, GATE3_OUTPUT);
  gatePass[3] = testGate(4, GATE4_INPUTS, GATE4_OUTPUT);

  // Summary
  Serial.println(F("\n========== SUMMARY =========="));
  bool chipOk = true;
  for (int g = 0; g < 4; g++) {
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
