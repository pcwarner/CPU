// Used to set clock prescaler
#include <avr/power.h>

const byte DATA_PINS[8] = {3, 4, 5, 6, 7, 8, 9, 10};
const byte CLOCK_PINS[5] = { A0, A1, A2, A4, 11 };
#define CIN_PIN 12
#define LED_PIN 13

const int  MASKS[8] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };
int count = 0;
bool ledOn = false;

void setData(int value) {
  for (int i = 0; i < 8; i++) {
    if ((value & MASKS[i]) == 0) {
      digitalWrite(DATA_PINS[i], LOW);
    } else {
      digitalWrite(DATA_PINS[i], HIGH);
    }
  }
  return;
}

void setClock(int value) {
  for (int i = 0; i < 5; i++) {
    if ((value & MASKS[i]) == 0) {
      digitalWrite(CLOCK_PINS[i], LOW);
    } else {
      digitalWrite(CLOCK_PINS[i], HIGH);
    }
  }
  return;
}

void setCarryIn(int value) {
  if (value == 0) {
    digitalWrite(CIN_PIN, LOW);
  } else {
    digitalWrite(CIN_PIN, HIGH);
  }
}

void setup() {
  clock_prescale_set(clock_div_2); // Uncomment when running at 32MHz
  Serial.begin(115200);

// Set the pin direction
  for (int i=0; i<8; i++) {
    pinMode(DATA_PINS[i], OUTPUT);
  }
  for (int i=0; i<5; i++) {
    pinMode(CLOCK_PINS[i], OUTPUT);
  }
  pinMode(CIN_PIN, OUTPUT);

  // Short delay for things to initalize
  delay(100);
  setData(0);
}

void loop() {
  Serial.println("Loop...");
   setClock(count);
   count++;
   if (count >= 32) {
    count = 0;
   }
    setData(0x55);
  delay(100);
  setData(0xAA);
  delay(100);
  ledOn = !ledOn;
  digitalWrite(LED_PIN, ledOn);
  digitalWrite(CIN_PIN, ledOn);
  //  for (int i=0; i<255; i++){
  //    setData(i);
  //    delay(100);
  // }
  // setData(0);
  //  for (int i=0; i<32; i++){
  //    setClock(i);
  //    delay(100);
  // }
  // setClock(0);
  // for (int i=0; i<10; i++){
  //    setCarryIn(1);
  //    delay(200);
  //    setCarryIn(0);
  //    delay(200);
  // }
}
