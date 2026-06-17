// ============================================================
//  74LS13N 2x 4-Input NAND Gate Tester
//  Arduino Nano
//
//  Wiring:
//   Gate 1 inputs : D2(1A)[1], D3(1B)[2], D4(1C)[4], D5(1D)[5]   output: A0(1Y)[6]
// chip pins [3] and [11] are not used
//   Gate 2 inputs : D6(2A)[9], D7(2B)[10], D8(2C)[12], D9(2D)[13]   output: A1(2Y)[8]
//   VCC  -> 5V[14]  |  GND -> GND[7]
//
//
//  Results are printed over Serial
// ============================================================

// --- Pin definitions ---
const int GATE1_INPUTS[4] = {2, 3, 4, 5};   
const int GATE2_INPUTS[4] = {6, 7, 8, 9};   

const int GATE1_OUTPUT = A0;  // Y1
const int GATE2_OUTPUT = A1;  // Y2


// Collect results
bool gatePass[2];
int  failCombo[2];   // first failing combo index (-1 = none)

// ---------------------------------------------------------------
// Apply a 3-bit combo (0..7) to three output pins
// ---------------------------------------------------------------
void applyInputs(const int pins[3], uint8_t combo) {
  digitalWrite(pins[0], (combo >> 3) & 1);
  digitalWrite(pins[1], (combo >> 2) & 1);
  digitalWrite(pins[2], (combo >> 1) & 1);
  digitalWrite(pins[3], (combo >> 0) & 1);
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
bool expectedNAND_4input(uint8_t combo){
  return (combo != 15);
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
  Serial.println(F(" A  B  C D | Y_actual | Y_expect | result"));

  bool allOk = true;
  int firstFail = -1;

  for (uint8_t combo = 0; combo < 16; combo++) {
    applyInputs(inputPins, combo);
    delay(100);
    int a       = (combo >> 3) & 1;
    int b       = (combo >> 2) & 1;
    int c       = (combo >> 1) & 1;
    int d       = (combo >> 0) & 1;
    int actual  = digitalRead(outputPin);
    int expected = expectedNAND_4input(combo) ? 1 : 0;
    bool ok      = (actual == expected);

    if (!ok && allOk) {          // record first failure
      allOk = false;
      firstFail = combo;
    }

    // Print row
    Serial.print(F(" "));
    Serial.print(a); Serial.print(F("  "));
    Serial.print(b); Serial.print(F("  "));
    Serial.print(c); Serial.print(F("  "));
    Serial.print(d); Serial.print(F(" |    "));
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
  for (int i = 0; i < 2; i++) {
    pinMode(GATE1_INPUTS[i], OUTPUT); digitalWrite(GATE1_INPUTS[i], LOW);
    pinMode(GATE2_INPUTS[i], OUTPUT); digitalWrite(GATE2_INPUTS[i], LOW);
  }

  // Configure output sense pins as INPUTs
  pinMode(GATE1_OUTPUT, INPUT_PULLUP);
  pinMode(GATE2_OUTPUT, INPUT_PULLUP);

  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS13  NAND Gate Tester"));
  Serial.println(F("========================================"));

  // Run tests
  gatePass[0] = testGate(1, GATE1_INPUTS, GATE1_OUTPUT);
  gatePass[1] = testGate(2, GATE2_INPUTS, GATE2_OUTPUT);


  // Summary
  Serial.println(F("\n========== SUMMARY =========="));
  bool chipOk = true;
  for (int g = 0; g < 2; g++) {
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
