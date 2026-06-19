/*
  74LS151N (8-to-1 Data Selector / Multiplexer) Test Sketch - Arduino Nano

  ----------------------- WIRING INSTRUCTIONS -----------------------

  74LS151N DIP-16 pinout:

  POWER:
  Arduino 5V ---------------------> VCC [16]
  Arduino GND ---------------------> GND [8]

  ENABLE (active LOW):
  D5 (ENABLE) [Arduino] -----------> /G [7]
  (Set LOW to enable the chip)

  SELECT LINES:
  D2 (A - LSB) [Arduino] ----------> A [11]
  D3 (B) [Arduino] ----------------> B [10]
  D4 (C - MSB) [Arduino] ---------> C [9]

  DATA INPUTS:
  D6  (I0) [Arduino] -------------> I0 [4]
  D7  (I1) [Arduino] -------------> I1 [3]
  D8  (I2) [Arduino] -------------> I2 [2]
  D9  (I3) [Arduino] -------------> I3 [1]
  D10 (I4) [Arduino] -------------> I4 [15]
  D11 (I5) [Arduino] -------------> I5 [14]
  D12 (I6) [Arduino] -------------> I6 [13]
  D13 (I7) [Arduino] -------------> I7 [12]

  OUTPUT:
  A0 (READ OUTPUT) [Arduino] <----- Y [5]

  NOTE:
  - W (inverted output) [6] is not used in this test.
  - This sketch drives inputs HIGH/LOW and verifies mux routing.
  - Expected behavior: Y should match selected input (I0–I7)

--------------------------------------------------------------------
*/

const int selA = 2;
const int selB = 3;
const int selC = 4;

const int enablePin = 5;

const int inputs[8] = {6, 7, 8, 9, 10, 11, 12, 13};

const int outputPin = A0;

void setSelect(int index) {
  digitalWrite(selA, index & 0x01);
  digitalWrite(selB, (index >> 1) & 0x01);
  digitalWrite(selC, (index >> 2) & 0x01);
}

void setInputs(int activeIndex) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(inputs[i], (i == activeIndex) ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(selA, OUTPUT);
  pinMode(selB, OUTPUT);
  pinMode(selC, OUTPUT);

  pinMode(enablePin, OUTPUT);

  for (int i = 0; i < 8; i++) {
    pinMode(inputs[i], OUTPUT);
  }

  pinMode(outputPin, INPUT);

  digitalWrite(enablePin, LOW); // enable 74LS151

  Serial.println("74LS151N MUX TEST START");
  Serial.println("Format: SELECT | INPUT STATE | OUTPUT | EXPECTED | RESULT");
  Serial.println("------------------------------------------------------------");
}

void loop() {
  for (int i = 0; i < 8; i++) {

    setSelect(i);
    setInputs(i);

    delay(5); // allow signals to settle

    int out = digitalRead(outputPin);
    int expected = HIGH; // selected input is HIGH

    bool pass = (out == expected);

    Serial.print("SEL=");
    Serial.print(i);

    Serial.print(" | I");
    Serial.print(i);
    Serial.print("=1 others=0");

    Serial.print(" | OUT=");
    Serial.print(out);

    Serial.print(" | EXPECT=");
    Serial.print(expected);

    Serial.print(" | ");
    Serial.println(pass ? "PASS" : "FAIL");
  }

  Serial.println("------------------------------------------------------------");
  delay(1000);
}