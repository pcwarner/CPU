#include <avr/power.h>

// Define pins used for the shift register
const int SHIFT_ENABLE = A0;  // Move buffer to register pin
const int SHIFT_CLK = A1;     // Shift register clock pin
const int SHIFT_DATA = A2;    // Shift register data pin
const int DATA_BIT_0 = 5;   // Data bit 0 pin
const int DATA_BIT_1 = 6;   // Data bit 1 pin
const int DATA_BIT_2 = 7;   // Data bit 2 pin
const int DATA_BIT_3 = 8;   // Data bit 3 pin
const int DATA_BIT_4 = 9;   // Data bit 4 pin
const int DATA_BIT_5 = 10;  // Data bit 5 pin
const int DATA_BIT_6 = 11;  // Data bit 6 pin
const int DATA_BIT_7 = 12;  // Data bit 7 pin
// #define RST_IN_PIN 11
// #define IRQ_IN_PIN 12
#define NUMBER_OF_DATA_PINS 8
const uint8_t DATA_PINS[NUMBER_OF_DATA_PINS] = { 5, 6, 7, 8, 9, 10, 11, 12 };
// int lastPortB = -1;
// int lastPortD = -1;
// int counter = 0;
// int lastValue = -1;
// bool valueChanged = false;
// uint32_t lastIrqSet = 0;
// uint32_t lastIrqSupClr = 0;
// uint32_t lastIrqSupSet = 0;
// bool clkClr = false;
// bool irqWaitClr = false;
// bool irqSupClr = false;
// bool irqSupSet = false;
// bool irqSet = false;
char command = 0;
uint8_t regA = 0;
uint8_t regB = 0;

void setup() {
  clock_prescale_set(clock_div_2);
  Serial.begin(115200);
  while(!Serial);
  delay(500);
  Serial.println("Starting...");
  for (int i=0; i < NUMBER_OF_DATA_PINS; i++) {
    pinMode(DATA_PINS[i], OUTPUT);
  }

  pinMode(SHIFT_ENABLE, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);

  digitalWrite(DATA_BIT_2, LOW); // M
  digitalWrite(DATA_BIT_3, LOW); // C
  digitalWrite(DATA_BIT_4, LOW); // S0
  digitalWrite(DATA_BIT_5, LOW); // S1
  digitalWrite(DATA_BIT_6, LOW); // S2
  digitalWrite(DATA_BIT_7, LOW); // S3
  regA = 0;
  regB = 0;
  setShiftOut();
}


void setShiftOut() {
  digitalWrite(SHIFT_ENABLE, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, regA);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, regB);
  digitalWrite(SHIFT_ENABLE, HIGH);
  return;
}

void checkCommand() {
  if (Serial.available()) {
    byte bytes[1];
    int numRead = Serial.readBytes(bytes, 1);
    if (numRead > 0) {
      command = toupper(bytes[0]);
      switch(command) {
        case '0':
          Serial.println("0: DEC A with A=0 M=HIGH S=LOW LOW LOW LOW");
          regA = 0;
          regB = 0;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW); // M
          digitalWrite(DATA_BIT_3, LOW); // C
          digitalWrite(DATA_BIT_7, LOW); // S3
          digitalWrite(DATA_BIT_6, LOW); // S2
          digitalWrite(DATA_BIT_5, LOW); // S1
          digitalWrite(DATA_BIT_4, LOW); // S0
          break;
        case '1':
          Serial.println("1: NOT A M=HIGH S=LOW LOW LOW LOW");
          regA = 0x55;
          regB = 0;
          setShiftOut();
          digitalWrite(DATA_BIT_2, HIGH); // M
          digitalWrite(DATA_BIT_3, LOW);  // C
          digitalWrite(DATA_BIT_7, LOW);  // S3
          digitalWrite(DATA_BIT_6, LOW);  // S2
          digitalWrite(DATA_BIT_5, LOW);  // S1
          digitalWrite(DATA_BIT_4, LOW);  // S0
          break;
        case '2':
          Serial.println("2: NOT B M=HIGH S=LOW HIGH LOW HIGH");
          regA = 0;
          regB = 0xaa;
          setShiftOut();
          digitalWrite(DATA_BIT_2, HIGH); // M
          digitalWrite(DATA_BIT_3, LOW);  // C
          digitalWrite(DATA_BIT_7, LOW);  // S3
          digitalWrite(DATA_BIT_6, HIGH); // S2
          digitalWrite(DATA_BIT_5, LOW);  // S1
          digitalWrite(DATA_BIT_4, HIGH); // S0
          break;
        case '3':
          Serial.println("3: OR M=HIGH S=HIGH LOW HIGH HIGH");
          regA = 0x63;
          regB = 0x49;
          setShiftOut();
          digitalWrite(DATA_BIT_2, HIGH); // M
          digitalWrite(DATA_BIT_3, LOW);  // C
          digitalWrite(DATA_BIT_7, HIGH); // S3
          digitalWrite(DATA_BIT_6, LOW);  // S2
          digitalWrite(DATA_BIT_5, HIGH); // S1
          digitalWrite(DATA_BIT_4, HIGH); // S0
          break;   
        case '4':
          Serial.println("4: AND M=HIGH S=HIGH HIGH HIGH LOW");
          regA = 0x63;
          regB = 0x49;
          setShiftOut();
          digitalWrite(DATA_BIT_2, HIGH); // M
          digitalWrite(DATA_BIT_3, LOW);  // C
          digitalWrite(DATA_BIT_7, HIGH); // S3
          digitalWrite(DATA_BIT_6, HIGH); // S2
          digitalWrite(DATA_BIT_5, HIGH); // S1
          digitalWrite(DATA_BIT_4, LOW);  // S0
          break;
          // ADD
        case '5':
          Serial.println("5: ADD 57 + 23 = 80 M=LOW S=HIGH LOW LOW HIGH");
          regA = 57;
          regB = 23;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW);  // M
          digitalWrite(DATA_BIT_3, LOW);  // C
          digitalWrite(DATA_BIT_7, HIGH); // S3
          digitalWrite(DATA_BIT_6, LOW);  // S2
          digitalWrite(DATA_BIT_5, LOW);  // S1
          digitalWrite(DATA_BIT_4, HIGH); // S0
          break;
        case '6':
          Serial.println("6: ADD 57 + 23 + Carry = 81 M=LOW S=HIGH LOW LOW HIGH");
          regA = 57;
          regB = 23;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW);  // M
          digitalWrite(DATA_BIT_3, HIGH); // C
          digitalWrite(DATA_BIT_7, HIGH); // S3
          digitalWrite(DATA_BIT_6, LOW);  // S2
          digitalWrite(DATA_BIT_5, LOW);  // S1
          digitalWrite(DATA_BIT_4, HIGH); // S0
          break;                                
        case '7':
          Serial.println("7: ADD 127 + 200 + Carry = 72 + Carry out M=LOW S=HIGH LOW LOW HIGH");
          regA = 127;
          regB = 200;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW);  // M
          digitalWrite(DATA_BIT_3, HIGH); // C
          digitalWrite(DATA_BIT_7, HIGH); // S3
          digitalWrite(DATA_BIT_6, LOW);  // S2
          digitalWrite(DATA_BIT_5, LOW);  // S1
          digitalWrite(DATA_BIT_4, HIGH); // S0
          break;
          // SUB
        case '8':
          Serial.println("8: SUB 170 - 10 + Carry = 160 M=LOW S=LOW HIGH HIGH LOW A > B so COUT=HIGH EQ=LOW");
          regA = 0x55;
          regB = 0x05;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW);  // M
          digitalWrite(DATA_BIT_3, HIGH); // C
          digitalWrite(DATA_BIT_7, LOW);  // S3
          digitalWrite(DATA_BIT_6, HIGH); // S2
          digitalWrite(DATA_BIT_5, HIGH); // S1
          digitalWrite(DATA_BIT_4, LOW);  // S0
          break;
        case '9':
          Serial.println("9: SUB 170 - 170 + Carry = 0 M=LOW S=LOW HIGH HIGH LOW A = B so COUT=HIGH EQ=HIGH");
          regA = 0x55;
          regB = 0x55;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW);  // M
          digitalWrite(DATA_BIT_3, HIGH); // C
          digitalWrite(DATA_BIT_7, LOW);  // S3
          digitalWrite(DATA_BIT_6, HIGH); // S2
          digitalWrite(DATA_BIT_5, HIGH); // S1
          digitalWrite(DATA_BIT_4, LOW);  // S0
          break;
        case 'A':
          Serial.println("A: SUB 10 - 52 + Carry = -42 M=LOW S=LOW HIGH HIGH LOW A = B so COUT=LOW EQ=LOW 11010110");
          regA = 10;
          regB = 52;
          setShiftOut();
          digitalWrite(DATA_BIT_2, LOW);  // M
          digitalWrite(DATA_BIT_3, HIGH); // C
          digitalWrite(DATA_BIT_7, LOW);  // S3
          digitalWrite(DATA_BIT_6, HIGH); // S2
          digitalWrite(DATA_BIT_5, HIGH); // S1
          digitalWrite(DATA_BIT_4, LOW);  // S0
          break;
       }
      command = 0;
    }
  }
}

void loop() {
  checkCommand();
}
