#include <avr/power.h>

// Define pins used for the shift register
const int SHIFT_ENABLE = A0;  // Move buffer to register pin
const int SHIFT_CLK = A1;     // Shift register clock pin
const int SHIFT_DATA = A2;    // Shift register data pin
#define RST_IN_PIN 11
#define IRQ_IN_PIN 12
#define NUMBER_OF_CLK_PINS 5
const uint8_t CLK_PINS[NUMBER_OF_CLK_PINS] = { 5, 6, 7, 8, 9 };
int lastPortB = -1;
int lastPortD = -1;
int counter = 0;
int lastValue = -1;
bool valueChanged = false;
uint32_t lastIrqSet = 0;
uint32_t lastIrqSupClr = 0;
uint32_t lastIrqSupSet = 0;
bool clkClr = false;
bool irqWaitClr = false;
bool irqSupClr = false;
bool irqSupSet = false;
bool irqSet = false;
char command = 0;

void setup() {
  clock_prescale_set(clock_div_2);
  Serial.begin(115200);
  while(!Serial);
  delay(500);
  Serial.println("Starting...");
  for (int i=0; i < NUMBER_OF_CLK_PINS; i++) {
    pinMode(CLK_PINS[i], INPUT);
  }
  pinMode(RST_IN_PIN, INPUT);
  pinMode(IRQ_IN_PIN, INPUT);
  pinMode(SHIFT_ENABLE, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);
  reset();
}

void reset() {
  Serial.println("Reset");
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);
  irqWaitClr = false;
  irqSupClr = false;
  irqSupSet = false;
  irqSet = false;
  setShiftOut();
}

void checkReset() {
  if (digitalRead(RST_IN_PIN)) {
    while(digitalRead(RST_IN_PIN)); // Wait till pin low
    reset();
  }
}

void setShiftOut() {
  int high = 0;
  int low = 0;
  if (clkClr) {
    low = low | 0b00000001;
  }
  if (irqWaitClr) {
    low = low | 0b00000010;
  }
  if (irqSupClr) {
    low = low | 0b00000100;
  }
  if (irqSupSet) {
    low = low | 0b00001000;
  }
  if (irqSet) {
    low = low | 0b00010000;
  }
  // address = address & 0x0000ffff;
  // byte low = address % 0x0100;
  // byte high = (address - low) / 0x0100;
  digitalWrite(SHIFT_ENABLE, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, high);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, low);
  digitalWrite(SHIFT_ENABLE, HIGH);
  return;
}

void checkClk() {
  int portB = PINB & 0b00000011;
  int portD = PIND & 0b11100000;
  counter++;
  if (lastPortB != portB || lastPortD != portD) {
    lastPortB = portB;
    lastPortD = portD;
    valueChanged = true;
    counter = 0;
  }
  if (valueChanged && counter >= 10) {
    int value = 0;
    if (portD & 0b00100000) {
      value += 1;
    }
    if (portD & 0b01000000) {
      value += 2;
    }
    if (portD & 0b10000000) {
      value += 4;
    }
    if (portB & 0b00000001) {
      value += 8;
    }
    if (portB & 0b00000010) {
      value += 16;
    }
    Serial.print(value);
    if (digitalRead(IRQ_IN_PIN)) {
      Serial.println(" IRQ");
    } else {
      Serial.println("");
    }
    // if (lastValue >= 0) {
    //   if (!(value == 0 && lastValue == 31 || value - 1 == lastValue)) {
    //     Serial.println("Step Missing");
    //     Serial.println(value);
    //     Serial.println(lastValue);
    //     while(1);
    //   }
    // }
    lastValue = value;
    valueChanged = false;
    counter = 0;
    if (value == 0) {
      clkClr = false;
      setShiftOut();
    } else {
      if (value == 11) {
        clkClr = true;
        setShiftOut();
        lastValue = 31;
        Serial.println("ClkClr");
      }
    }
  }
}

void checkIrqSupClr() {
  if (command == 'C') {
    Serial.println("IRQSUP off");
    irqSupClr = true;
    setShiftOut();
    lastIrqSupClr = millis();
    command = 0;
  } else {
    if (irqSupClr) {
      if (millis() - lastIrqSupClr > 10) {
        irqSupClr = false;
        setShiftOut();
      }
    }
  }
}

void checkIrqSupSet() {
  if (command == 'S') {
    Serial.println("IRQSUP on");
    irqSupSet = true;
    setShiftOut();
    lastIrqSupSet = millis();
    command = 0;
  } else {
    if (irqSupSet) {
      if (millis() - lastIrqSupSet > 10) {
        irqSupSet = false;
        setShiftOut();
      }
    }
  }
}

void checkIrqSet() {
  if (command == 'I') {
    Serial.println("IRQ");
    irqSet = true;
    setShiftOut();
    lastIrqSet = millis();
    command = 0;
  } else {
    if (irqSet) {
      if (millis() - lastIrqSet > 10) {
        irqSet = false;
        setShiftOut();
      }
    }
  }
}

void checkCommand() {
  if (Serial.available()) {
    byte bytes[1];
    int numRead = Serial.readBytes(bytes, 1);
    if (numRead > 0) {
      command = toupper(bytes[0]);
      if (command == 'I' || command == 'S' || command == 'C') {
        Serial.println(command);
      }
    }
  }
}

void loop() {

  checkCommand();
 
  checkReset();

  checkClk();

  checkIrqSupClr();

  checkIrqSupSet();

  checkIrqSet();

}
