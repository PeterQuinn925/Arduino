// ============================================================
//  74LS04N 6x INVERTER Gate Tester
//  Arduino Nano
//
//  NEW Wiring: Arduino Pin (signal name) [chip pin number]
//   Gate 1 input : D2(1A)[1] output: D3(1Y)[2]
//   Gate 2 input : D4(2A)[3] output: A1(2Y)[4] changed for testing this gate
//   Gate 3 input : D6(3A)[5] output: D7(3Y)[6]
//   Gate 4 input : D8(4A)[9] output: D9(4Y)[8]
//   Gate 5 input : A0(5A)[11] output: D5(5Y)[10] changed for testing
//   Gate 6 input : A3(6A)[13] output: A4(6Y)[12]
//   VCC[14]  -> 5V  |  GND[7] -> GND
//
//
//  Results are printed over Serial
// ============================================================

// --- Pin definitions ---
const int GATE_INPUTS[6] = { 2, 4, 6, 8, A0, A3 };
const int GATE_OUTPUTS[6] = { 3, A1, 7, 9, 5, A4 };
int failed_gate = 9;

bool CheckGates(int value) {
  for (int i = 0; i < 6; i++) {
    digitalWrite(GATE_INPUTS[i], value);
  }
  delay(100);
  bool Passed = true;
  Serial.print(!value);
  Serial.println("s expected");
  for (int i = 0; i < 6; i++) {
    digitalWrite(GATE_INPUTS[i], value);
    int actual = digitalRead(GATE_OUTPUTS[i]);
    Serial.print(actual);
    Serial.print(" ");
    if (actual == value) { Passed = false; };
  }
  Serial.println();
  return Passed;
}


// ---------------------------------------------------------------
// setup
// ---------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for USB serial on Nano

  // Configure input driver pins as OUTPUTs, default LOW
  for (int i = 0; i < 6; i++) {
    pinMode(GATE_INPUTS[i], OUTPUT);
    digitalWrite(GATE_INPUTS[i], LOW);
    // Configure output sense pins as INPUTs
    pinMode(GATE_OUTPUTS[i], INPUT_PULLUP);
  }

  delay(100);  // let power rails stabilise

  Serial.println(F("\n========================================"));
  Serial.println(F("  74LS04N 6x Inverter Gate Tester"));
  Serial.println(F("========================================"));

  // Run tests
  CheckGates(0);
  CheckGates(1);

  //Change each bit in sequence
  bool failed = false;
  uint8_t bits[6];
  int actual;
  for (int n = 0; n < 64; n++) {
    for (int i = 5; i >= 0; i--) {
      bits[i] = (n >> (5 - i)) & 1;
      digitalWrite(GATE_INPUTS[i], bits[i]);
      delay(100);
      actual = digitalRead(GATE_OUTPUTS[i]);
      if (actual == bits[i]) {
        Serial.println("**Failed**");
        Serial.println(n);
        failed = true;
        failed_gate = i;
        break;
      }
    }
    if (failed) break;
  }
  if (!failed) Serial.println("Good chip!");
}

// ---------------------------------------------------------------
// loop — nothing to do, all work done in setup
// ---------------------------------------------------------------
void loop() {
  Serial.println(failed_gate);
  Serial.println("-----Expect High-----");
  delay(5000);
  if (failed_gate != 9) {
    int n = 100;
    float volts[n];
    pinMode(GATE_OUTPUTS[failed_gate], INPUT);
    digitalWrite(GATE_INPUTS[failed_gate], 0);
        Serial.println("t2");
    for (int i = 0; i < n; i++) {
      volts[i] = analogRead(GATE_OUTPUTS[failed_gate]);
    }

    for (int i = 0; i < n; i++) {
      Serial.println(volts[i]* (5.0 / 1023.0));
    }
    Serial.println("-----Expect Low----");
    digitalWrite(GATE_INPUTS[failed_gate], 0);
    for (int i = 0; i < n; i++) {
      volts[i] = analogRead(GATE_OUTPUTS[failed_gate]);
    }
    for (int i = 0; i < n; i++) {
      Serial.println(volts[i]* (5.0 / 1023.0));
    }
    //delay(100);
    //actual = digitalRead(GATE_OUTPUTS[failed_gate]);
    //Serial.println(actual);
  }
}
