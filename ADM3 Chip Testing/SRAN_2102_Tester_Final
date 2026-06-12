// ============================================================
//  MM2102AN SRAM Tester — final clean version
//
//  2102 Pin  Signal   Nano Pin
//  --------  ------   --------
//   1        A6       D10
//   2        A5       D9
//   3        A4       D8
//   4        A3       D7
//   5        WR       A3  (active LOW)
//   6        A1       D5
//   7        A2       D6
//   8        A0       D4
//   9        GND      GND
//  10        VCC      5V
//  11        DI       A5
//  12        DO       D3  + 1kΩ to 5V
//  13        CE       A4  (active LOW, held LOW)
//  14        A9       D13
//  15        A8       D12
//  16        A7       D11
// ============================================================

#define CE_pin       A4
#define RW_pin       A3
#define data_DO_pin   3
#define data_DI_pin  A5

const uint8_t ADDR_PINS[10] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
// index:                       A0 A1 A2 A3 A4 A5 A6  A7  A8  A9

const uint16_t MEM_SIZE = 1024;

void setAddress(uint16_t addr) {
  for (uint8_t i = 0; i < 10; i++)
    digitalWrite(ADDR_PINS[i], (addr >> i) & 1);
  delayMicroseconds(20);
}

void writeCell(uint16_t addr, uint8_t bit) {
  setAddress(addr);
  digitalWrite(data_DI_pin, bit & 1);
  //Serial.print(bit & 1);
  delayMicroseconds(30);
  digitalWrite(RW_pin, LOW);
  delayMicroseconds(30);
  digitalWrite(RW_pin, HIGH);
  delayMicroseconds(30);
}

uint8_t readCell(uint16_t addr) {
  setAddress(addr);
  delayMicroseconds(30);
  //Serial.print(digitalRead(data_DO_pin));
  return digitalRead(data_DO_pin);
}

uint16_t testPattern(uint8_t bit, const char *label) {
  uint16_t errors = 0;
  Serial.print(F("  All-")); Serial.print(label); Serial.print(F(" ... "));
  for (uint16_t a = 0; a < MEM_SIZE; a++) writeCell(a, bit);
  for (uint16_t a = 0; a < MEM_SIZE; a++) {
    if (readCell(a) != bit) {
      if (!errors) Serial.println();
      Serial.print(F("    FAIL addr=0x")); Serial.println(a, HEX);
      errors++;
      if (errors >= 16) { Serial.println(F("    (too many, stopping)")); break; }
    }
  }
  if (!errors) Serial.println(F("PASS"));
  return errors;
}

uint16_t testCheckerboard() {
  uint8_t one = 1;
  uint8_t zero = 0; 
  uint16_t errors = 0;
  Serial.print(F("  Checkerboard ... "));
  for (uint16_t a = 0; a < MEM_SIZE/2; a=a+2) {
    writeCell(a, one);
    writeCell(a+1,zero);
    }

  for (uint16_t a = 0; a < MEM_SIZE/2; a=a+2) {
    uint8_t expected = one;
    uint8_t expected1 = zero;
    uint8_t got = readCell(a);
    uint8_t got1 = readCell(a+1);
    if (got != expected or got1 != expected1) {
      if (!errors) Serial.println();
      Serial.print(F("    FAIL addr=0x")); Serial.print(a, HEX);
      Serial.print(F(" exp=")); Serial.print(expected);
      Serial.print(F(" got=")); Serial.println(got);
      errors++;
      if (errors >= 16) { Serial.println(F("    (too many, stopping)")); break; }
    }
  }
  if (!errors) Serial.println(F("PASS"));
  return errors;
}

uint16_t testWalking1() {
  uint16_t errors = 0;
  Serial.print(F("  Walking-1 (slow) ... "));
  for (uint16_t a = 0; a < MEM_SIZE; a++) writeCell(a, 0);
  for (uint16_t hot = 0; hot < MEM_SIZE; hot++) {
    writeCell(hot, 1);
    for (uint16_t a = 0; a < MEM_SIZE; a++) {
      uint8_t expected = (a == hot) ? 1 : 0;
      if (readCell(a) != expected) {
        if (!errors) Serial.println();
        Serial.print(F("    FAIL hot=0x")); Serial.print(hot, HEX);
        Serial.print(F(" addr=0x")); Serial.println(a, HEX);
        errors++;
        if (errors >= 8) { Serial.println(F("    (too many, stopping)")); return errors; }
      }
    }
    writeCell(hot, 0);
  }
  if (!errors) Serial.println(F("PASS"));
  return errors;
}

uint16_t testAddressLines() {
  uint16_t errors = 0;
  Serial.print(F("  Address lines ... "));
  for (uint8_t i = 0; i < 10; i++) writeCell((uint16_t)1 << i, i & 1);
  for (uint8_t i = 0; i < 10; i++) {
    uint8_t expected = i & 1;
    uint8_t got = readCell((uint16_t)1 << i);
    if (got != expected) {
      if (!errors) Serial.println();
      Serial.print(F("    FAIL A")); Serial.print(i);
      Serial.print(F(" exp=")); Serial.print(expected);
      Serial.print(F(" got=")); Serial.println(got);
      errors++;
    }
  }
  if (!errors) Serial.println(F("PASS"));
  return errors;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  pinMode(CE_pin,  OUTPUT); digitalWrite(CE_pin,  LOW);
  pinMode(RW_pin,  OUTPUT); digitalWrite(RW_pin,  HIGH);
  for (uint8_t i = 0; i < 10; i++) { pinMode(ADDR_PINS[i], OUTPUT); digitalWrite(ADDR_PINS[i], LOW); }
  pinMode(data_DI_pin, OUTPUT); digitalWrite(data_DI_pin, LOW);
  pinMode(data_DO_pin, INPUT);

  delay(50);

  Serial.println(F("========================================"));
  Serial.println(F("  MM2102AN SRAM Tester"));
  Serial.println(F("========================================"));
  Serial.println();

  uint16_t totalErrors = 0;

  Serial.println(F("[ 1 ] Pattern tests"));
  totalErrors += testPattern(0, "0");
  totalErrors += testPattern(1, "1");

  Serial.println(F("[ 2 ] Checkerboard"));
  totalErrors += testCheckerboard();

  Serial.println(F("[ 3 ] Address line test"));
  totalErrors += testAddressLines();

  Serial.println(F("[ 4 ] Walking-1 test"));
  totalErrors += testWalking1();

  Serial.println();
  Serial.println(F("========================================"));
  if (totalErrors == 0) {
    Serial.println(F("  RESULT: PASS -- chip is good!"));
  } else {
    Serial.print(F("  RESULT: FAIL -- "));
    Serial.print(totalErrors);
    Serial.println(F(" errors found."));
  }
  Serial.println(F("  Press RESET to run again."));
  Serial.println(F("========================================"));
}

void loop() {}
