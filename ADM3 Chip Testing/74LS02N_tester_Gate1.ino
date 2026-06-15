// ============================================================
//  74LS02N 4x 2-Input Postive-NOR Gate Tester
//  Arduino Nano
//
//  OLD LS27 Wiring: Arduino Pin (signal name) [chip pin number]
//   Gate 1 inputs : D2(1A)[1], D3(1B)[2], D4(1C)[13]   output: A0(1Y)[12]
//   Gate 2 inputs : D5(2A)[3], D6(2B)[4], D7(2C)[5]   output: A1(2Y)[6]
//   Gate 3 inputs : D8(3A)[9], D9(3B)[10], D10(3C)[11]  output: A2(3Y)[8]
//   VCC  -> 5V[14]  |  GND -> GND[7]

//  NEW Wiring: Arduino Pin (signal name) [chip pin number]
//   Gate 1 inputs : D3(1A)[2], D5(1B)[3]   output: D2(1Y)[1]
//   Gate 2 inputs : D7(2A)[5], A1(2B)[6]   output: D6(2Y)[4]
//   Gate 3 inputs : A2(3A)[8], D8(3B)[9],  output: D9(3Y)[10]
//   Gate 4 inputs : D10(4A)[11], A0(4B)[12],  output: D4(4Y)[13]
//   VCC[14]  -> 5V  |  GND[7] -> GND
//
//
//  Results are printed over Serial (115200 baud).
// ============================================================

// --- Pin definitions ---
const int GATE1_INPUTS[2] = { 3, 5 };
const int GATE2_INPUTS[2] = { 7, A1 };
const int GATE3_INPUTS[2] = { A2, 8 };
const int GATE4_INPUTS[2] = { 10, A0 };

const int GATE1_OUTPUT = 2;  // Y1
const int GATE2_OUTPUT = 6;  // Y2
const int GATE3_OUTPUT = 9;  // Y3
const int GATE4_OUTPUT = 4;  // Y3

// Collect results
bool gatePass[4];
int failCombo[4];  // first failing combo index (-1 = none)

// ---------------------------------------------------------------
// Apply a 3-bit combo (0..7) to three output pins
// ---------------------------------------------------------------
void applyInputs(const int pins[3], uint8_t combo) {
  digitalWrite(pins[0], (combo >> 2) & 1);
  digitalWrite(pins[1], (combo >> 1) & 1);
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
    int a = (combo >> 1) & 1;
    int b = (combo >> 0) & 1;
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
    Serial.print(F(" |    "));
    Serial.print(actual);
    Serial.print(F("     |    "));
    Serial.print(expected);
    Serial.print(F("     | "));
    Serial.println(ok ? F("PASS") : F("FAIL <<<"));
    delay(1000);
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
  while (!Serial) {}  // wait for USB serial on Nano

  // Configure input driver pins as OUTPUTs, default LOW
  for (int i = 0; i < 2; i++) {
    pinMode(GATE1_INPUTS[i], OUTPUT);
    digitalWrite(GATE1_INPUTS[i], LOW);
    pinMode(GATE2_INPUTS[i], OUTPUT);
    digitalWrite(GATE2_INPUTS[i], LOW);
    pinMode(GATE3_INPUTS[i], OUTPUT);
    digitalWrite(GATE3_INPUTS[i], LOW);
    pinMode(GATE4_INPUTS[i], OUTPUT);
    digitalWrite(GATE4_INPUTS[i], LOW);
  }

  // Configure output sense pins as INPUTs
  pinMode(GATE1_OUTPUT, INPUT_PULLUP);
  pinMode(GATE2_OUTPUT, INPUT);
  pinMode(GATE3_OUTPUT, INPUT);
  pinMode(GATE4_OUTPUT, INPUT);

  delay(100);  // let power rails stabilise
}

// ---------------------------------------------------------------
// loop — nothing to do, all work done in setup
// ---------------------------------------------------------------
void loop() {
  int a = 0;
  int b = 0;
  Serial.print(a);
  Serial.print(" ");
  Serial.print(b); Serial.print(" ");
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  delay(100);
  int actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(500);
  /*
  a = 1;
  b = 0;
  Serial.print(a);
  Serial.print(" ");
  Serial.print(b); Serial.print(" ");
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(1000);
  a = 0;
  b = 1;
  Serial.print(a);
  Serial.print(" ");
Serial.print(b); Serial.print(" ");
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(1000);
  a = 1;
  b = 1;
  Serial.print(a);
  Serial.print(" ");
Serial.print(b); Serial.print(" ");
  digitalWrite(GATE1_INPUTS[0], a);
  digitalWrite(GATE1_INPUTS[1], b);
  delay(100);
  actual = digitalRead(GATE1_OUTPUT);
  Serial.println(actual);
  delay(1000);
  Serial.println("-----");
  */
}
