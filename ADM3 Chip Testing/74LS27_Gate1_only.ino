// ============================================================
//  74LS27N Triple 3-Input NOR Gate Tester
//  Arduino Nano
//
//  Wiring:
//   Gate 1 inputs : D2(A1), D3(B1), D4(C1)   output: A0(Y1)
//   Gate 2 inputs : D5(A2), D6(B2), D7(C2)   output: A1(Y2)
//   Gate 3 inputs : D8(A3), D9(B3), D10(C3)  output: A2(Y3)
//   VCC  -> 5V  |  GND -> GND
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
const int GATE1_INPUTS[3] = { 2, 3, 4 };   // A1, B1, C1
const int GATE2_INPUTS[3] = { 5, 6, 7 };   // A2, B2, C2
const int GATE3_INPUTS[3] = { 8, 9, 10 };  // A3, B3, C3

const int GATE1_OUTPUT = A0;  // Y1
const int GATE2_OUTPUT = A1;  // Y2
const int GATE3_OUTPUT = A2;  // Y3

// Collect results
bool gatePass[3];
int failCombo[3];  // first failing combo index (-1 = none)

// ---------------------------------------------------------------
// Apply a 3-bit combo (0..7) to three output pins
// ---------------------------------------------------------------
void applyInputs(const int pins[3], uint8_t combo) {
  digitalWrite(pins[0], (combo >> 2) & 1);
  digitalWrite(pins[1], (combo >> 1) & 1);
  digitalWrite(pins[2], (combo >> 0) & 1);
  delayMicroseconds(10);  // propagation delay headroom (~50 ns typical)
}

// ---------------------------------------------------------------
// Expected NOR output: HIGH only when all three inputs are LOW
// ---------------------------------------------------------------
bool expectedNOR(uint8_t combo) {
  return (combo == 0);
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
    delay(5000);
    int a = (combo >> 2) & 1;
    int b = (combo >> 1) & 1;
    int c = (combo >> 0) & 1;
    int actual = digitalRead(outputPin);
    int expected = expectedNOR(combo) ? 1 : 0;
    bool ok = (actual == expected);

    if (!ok && allOk) {  // record first failure
      allOk = false;
      firstFail = combo;
    }

    // Print row
    Serial.print(F(" "));
    Serial.print(a);
    Serial.print(F("  "));
    Serial.print(b);
    Serial.print(F("  "));
    Serial.print(c);
    Serial.print(F(" |    "));
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
void PrintBits(int a, int b, int c)
{
  Serial.print(a);
  Serial.print(" ");
  Serial.print(b);
  Serial.print(" ");
  Serial.print(c);
  Serial.print(" ");
}
// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for USB serial on Nano

  // Configure input driver pins as OUTPUTs, default LOW
  for (int i = 0; i < 3; i++) {
    pinMode(GATE1_INPUTS[i], OUTPUT);
    digitalWrite(GATE1_INPUTS[i], LOW);
    pinMode(GATE2_INPUTS[i], OUTPUT);
    digitalWrite(GATE2_INPUTS[i], LOW);
    pinMode(GATE3_INPUTS[i], OUTPUT);
    digitalWrite(GATE3_INPUTS[i], LOW);
  }

  // Configure output sense pins as INPUTs
  pinMode(GATE1_OUTPUT, INPUT);
  pinMode(GATE2_OUTPUT, INPUT);
  pinMode(GATE3_OUTPUT, INPUT);

  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS27N gate 1 only Tester"));
  Serial.println(F("========================================"));


}

// ---------------------------------------------------------------
// loop — nothing to do, all work done in setup
// ---------------------------------------------------------------
void loop() {
  // Run tests
  int a = 0;
  int b = 0;
  int c = 0;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  int actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(5000);
  a = 1;
  b = 0;
  c = 0;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(5000);
  a = 0;
  b = 1;
  c = 0;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(5000);
  a = 1;
  b = 1;
  c = 0;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(5000);
  a = 0;
  b = 0;
  c = 1;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(5000);
  a = 1;
  b = 0;
  c = 1;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(5000);
  a = 1;
  b = 1;
  c = 1;
  PrintBits(a, b, c);
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  digitalWrite(GATE1_INPUTS[2], c);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  Serial.println("-------");
  delay(5000);

}
