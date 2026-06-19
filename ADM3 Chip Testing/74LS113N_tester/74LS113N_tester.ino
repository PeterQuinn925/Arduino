// ============================================================
//  DM74LS113N / SN74LS113AN
//  Dual JK Negative Edge-Triggered Flip-Flop Tester
//  Arduino Nano
//
//  Wiring: Arduino pin (signal name) [chip pin]
//
//  Flip-Flop 1:
//   D2  (CLK)  [1]   Clock (falling-edge triggered)
//   D3  (1K)    [2]   K input
//   D4  (1J)    [3]   J input
//   D5  (1PRE)  [4]   Active LOW preset
//   A0  (1Q)    [5]   Q output
//   A1  (1QB)   [6]   /Q output
//
//  GND          [7]
//
//  Flip-Flop 2:
//   A3  (2QB)   [8]   /Q output
//   A2  (2Q)    [9]   Q output
//   D10 (2PRE)  [10]  Active LOW preset
//   D7  (2J)    [11]  J input
//   D9  (2K)    [12]  K input
//   D8  (2CLK)  [13]  Clock (falling-edge triggered)
//
//   VCC         [14]
//
//  NOTE:
//  - Clocked on HIGH->LOW transition.
//  - /PRE is asynchronous and active LOW.
//  - No asynchronous CLEAR on LS113.
// ============================================================

const int FF1_CLK = 2;
const int FF1_K   = 3;
const int FF1_J   = 4;
const int FF1_PRE = 5;
const int FF1_Q   = A0;
const int FF1_QB  = A1;

const int FF2_J   = 7;
const int FF2_CLK = 8;
const int FF2_K   = 9;
const int FF2_PRE = 10;
const int FF2_Q   = A2;
const int FF2_QB  = A3;

bool ffPass[2];

// ---------------------------------------------------------------
void pulseClock(int ffNum, int clkPin,
int qPin, int qbPin) {

Serial.print(F("  [CLK FF"));
Serial.print(ffNum);
Serial.print(F("] Before: Q="));
Serial.print(digitalRead(qPin));
Serial.print(F(" /Q="));
Serial.println(digitalRead(qbPin));

digitalWrite(clkPin, HIGH);
delay(40);

digitalWrite(clkPin, LOW);   // falling edge
delay(40);

digitalWrite(clkPin, HIGH);
delay(40);

Serial.print(F("  [CLK FF"));
Serial.print(ffNum);
Serial.print(F("] After : Q="));
Serial.print(digitalRead(qPin));
Serial.print(F(" /Q="));
Serial.println(digitalRead(qbPin));
}

// ---------------------------------------------------------------
void assertPreset(int ffNum,
int prePin,
int qPin,
int qbPin) {

Serial.print(F("  [PRE FF"));
Serial.print(ffNum);
Serial.print(F("] Assert LOW -> "));

digitalWrite(prePin, LOW);
delay(20);

Serial.print(F("Q="));
Serial.print(digitalRead(qPin));
Serial.print(F(" /Q="));
Serial.println(digitalRead(qbPin));

delay(50);
}

void releasePreset(int prePin) {
digitalWrite(prePin, HIGH);
delay(30);
}

// ---------------------------------------------------------------
bool checkOutputs(int qPin,
int qbPin,
int expectedQ,
const char* phase,
const char* mode) {

int q  = digitalRead(qPin);
int qb = digitalRead(qbPin);

bool ok =
(q == expectedQ) &&
(qb == !expectedQ);

Serial.print(F("  "));
Serial.print(phase);

int plen = strlen(phase);
for (int i = plen; i < 10; i++) Serial.print(' ');

Serial.print(F("  "));
Serial.print(mode);

int mlen = strlen(mode);
for (int i = mlen; i < 24; i++) Serial.print(' ');

Serial.print(F(" Q="));
Serial.print(q);
Serial.print(F(" /Q="));
Serial.print(qb);
Serial.print(F("  "));
Serial.println(ok ? F("PASS") : F("FAIL <<<"));

return ok;
}

// ---------------------------------------------------------------
// Force known state Q=1 via preset.
// ---------------------------------------------------------------
void presetFF(int prePin,
int clkPin,
int jPin,
int kPin) {

digitalWrite(clkPin, HIGH);
digitalWrite(jPin, LOW);
digitalWrite(kPin, LOW);

digitalWrite(prePin, LOW);
delay(50);

digitalWrite(prePin, HIGH);
delay(50);
}

// ---------------------------------------------------------------
bool testFF(int ffNum,
int pinJ,
int pinCLK,
int pinK,
int pinPRE,
int pinQ,
int pinQB) {

Serial.print(F("\n--- Flip-Flop "));
Serial.print(ffNum);
Serial.println(F(" ---"));

bool allOk = true;

// -----------------------------
// Async preset
// -----------------------------
assertPreset(ffNum, pinPRE, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Async", "PRE asserted");

releasePreset(pinPRE);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Async", "PRE released");

// -----------------------------
// Clocked reset J=0 K=1
// -----------------------------
digitalWrite(pinJ, LOW);
digitalWrite(pinK, HIGH);

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 0,
"Clocked", "J=0 K=1 RESET");

// -----------------------------
// Clocked set J=1 K=0
// -----------------------------
digitalWrite(pinJ, HIGH);
digitalWrite(pinK, LOW);

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Clocked", "J=1 K=0 SET");

// -----------------------------
// Hold at 1
// -----------------------------
digitalWrite(pinJ, LOW);
digitalWrite(pinK, LOW);

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Clocked", "J=0 K=0 HOLD1");

// -----------------------------
// Reset again
// -----------------------------
digitalWrite(pinJ, LOW);
digitalWrite(pinK, HIGH);

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 0,
"Clocked", "RESET");

// -----------------------------
// Hold at 0
// -----------------------------
digitalWrite(pinJ, LOW);
digitalWrite(pinK, LOW);

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 0,
"Clocked", "HOLD0");

// -----------------------------
// Toggle
// -----------------------------
digitalWrite(pinJ, HIGH);
digitalWrite(pinK, HIGH);

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Clocked", "TOGGLE 0->1");

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 0,
"Clocked", "TOGGLE 1->0");

pulseClock(ffNum, pinCLK, pinQ, pinQB);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Clocked", "TOGGLE 0->1");

// -----------------------------
// No clock edge
// -----------------------------
digitalWrite(pinJ, LOW);
digitalWrite(pinK, HIGH);

delay(100);

allOk &= checkOutputs(
pinQ, pinQB, 1,
"Hold", "No CLK edge");

return allOk;
}

// ---------------------------------------------------------------
void setup() {

Serial.begin(9600);
while (!Serial) {}

digitalWrite(FF1_PRE, HIGH);
digitalWrite(FF1_CLK, HIGH);
digitalWrite(FF1_J, LOW);
digitalWrite(FF1_K, LOW);

pinMode(FF1_PRE, OUTPUT);
pinMode(FF1_CLK, OUTPUT);
pinMode(FF1_J, OUTPUT);
pinMode(FF1_K, OUTPUT);

pinMode(FF1_Q, INPUT);
pinMode(FF1_QB, INPUT);

digitalWrite(FF2_PRE, HIGH);
digitalWrite(FF2_CLK, HIGH);
digitalWrite(FF2_J, LOW);
digitalWrite(FF2_K, LOW);

pinMode(FF2_PRE, OUTPUT);
pinMode(FF2_CLK, OUTPUT);
pinMode(FF2_J, OUTPUT);
pinMode(FF2_K, OUTPUT);

pinMode(FF2_Q, INPUT);
pinMode(FF2_QB, INPUT);

delay(200);

Serial.println(F("\n========================================"));
Serial.println(F(" DM74LS113N Dual JK Flip-Flop Tester"));
Serial.println(F(" Falling-edge triggered"));
Serial.println(F("========================================"));

ffPass[0] = testFF(
1,
FF1_J,
FF1_CLK,
FF1_K,
FF1_PRE,
FF1_Q,
FF1_QB);

ffPass[1] = testFF(
2,
FF2_J,
FF2_CLK,
FF2_K,
FF2_PRE,
FF2_Q,
FF2_QB);

bool chipOk = ffPass[0] && ffPass[1];

Serial.println(F("\n========== SUMMARY =========="));

Serial.print(F("Flip-Flop 1: "));
Serial.println(ffPass[0] ? F("PASS") : F("FAIL"));

Serial.print(F("Flip-Flop 2: "));
Serial.println(ffPass[1] ? F("PASS") : F("FAIL"));

Serial.println(F("-----------------------------"));
Serial.println(chipOk ? F("Chip: GOOD") : F("Chip: FAULTY"));
Serial.println(F("============================="));
Serial.println(F("\nPress reset to test again."));
}

void loop() {}
