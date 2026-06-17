// ============================================================
//  74LS42N  4-LINE BCD TO 10-LINE DECIMAL DECODER Tester
//  Arduino Nano
//
//  Wiring: 16 pin package. [chip pin] (signal name) Arduino pin
//  Outputs (backwards from previous!)
//  [1](0) D2
//  [2](1) D3
//  [3](2) D4
//  [4](3) D5
//  [5](4) D6
//  [6](5) D7
//  [7](6) D8
//  [8] GND
//  [9](7) D9
//  [10](8) D10
//  [11](9) D11
//  Inputs
//  [12](D) A3
//  [13](C) A2
//  [14](B) A1
//  [15](A) A0
//  [16] VCC
//
//
//  Results are printed over Serial
// ============================================================

// --- Pin definitions ---
const int GATE_INPUTS[4] = {A0, A1, A2, A3};    
const int GATE_OUTPUTS[10] = {2, 3, 4, 5,6,7,8,9,10,11};


// Collect results
bool gatePass;
int  failCombo;   // first failing combo index (-1 = none)


// ---------------------------------------------------------------
// Expected NOR output: HIGH only when all three inputs are LOW
// ---------------------------------------------------------------
// Returns true if output n should be LOW (active) for a given 4-bit combo
bool expectedBCD(uint8_t combo, int outputNum) {
    if (combo > 9) return false;      // invalid BCD — all outputs HIGH
    return (combo == outputNum);      // only the matching output goes LOW
}
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
void testGate() {
  bool anyFailed = false; 
  Serial.println(F(" ---"));
  Serial.println(F(" A  B  C D | Y_actual | Y_expect | result"));

for (uint8_t combo = 0; combo < 16; combo++) {
    // set the 4 input pins (D, C, B, A)
digitalWrite(GATE_INPUTS[0], (combo >> 0) & 1);  // A (LSB)
digitalWrite(GATE_INPUTS[1], (combo >> 1) & 1);  // B
digitalWrite(GATE_INPUTS[2], (combo >> 2) & 1);  // C
digitalWrite(GATE_INPUTS[3], (combo >> 3) & 1);  // D (MSB)
    delay(10);

    for (int out = 0; out < 10; out++) {
        int actual   = digitalRead(GATE_OUTPUTS[out]);
        int expected = expectedBCD(combo, out) ? 0 : 1;  // active LOW
        bool ok      = (actual == expected);

        if (!ok) {
            anyFailed = true; 
            Serial.print(F("FAIL combo="));
            Serial.print(combo);
            Serial.print(F(" output=Y"));
            Serial.println(out);
        }
    }
}
Serial.println(anyFailed ? F("Chip: FAULTY") : F("Chip: GOOD"));

  // Drive all inputs LOW after test
    digitalWrite(GATE_INPUTS[0], 0);
    digitalWrite(GATE_INPUTS[1], 0);
    digitalWrite(GATE_INPUTS[2], 0);
    digitalWrite(GATE_INPUTS[3], 0);
}

// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}            // wait for USB serial on Nano

  // Configure input driver pins as OUTPUTs, default LOW
  for (int i = 0; i < 4; i++) {
    pinMode(GATE_INPUTS[i], OUTPUT); digitalWrite(GATE_INPUTS[i], LOW);
  }

  // Configure output sense pins as INPUTs
   for (int i = 0; i < 10; i++) {
      pinMode(GATE_OUTPUTS[i], INPUT);
  }
 
  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS42  NAND Gate Tester"));
  Serial.println(F("========================================"));

  // Run tests
  testGate();

  Serial.println(F("============================="));
  Serial.println(F("\nPress reset to test again."));
}

// ---------------------------------------------------------------
// loop — nothing to do, all work done in setup
// ---------------------------------------------------------------
void loop() {}
