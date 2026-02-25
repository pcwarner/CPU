#include <avr/power.h>

// Define pins used for the shift register
const int SHIFT_ENABLE = A0;  // Move buffer to register pin
const int SHIFT_CLK = A1;     // Shift register clock pin
const int SHIFT_DATA = A2;    // Shift register data pin
const int PAUSE = 10;
const int OUTPUT_ENABLE = A3;
const int WRITE_ENABLE = A4;
const int LED = 13;
#define NUMBER_OF_DATA_PINS 8
const uint8_t DATA_PINS[NUMBER_OF_DATA_PINS] = { 2, 3, 4, 5, 6, 7, 8, 9 };
const int DATA_MASKS[NUMBER_OF_DATA_PINS] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };

#define CODE_SIZE 5
const uint8_t CODE[CODE_SIZE] = {
      0x00,  // NULL
      0x00,  // NULL
      0x80,0x00,0x00 // JMP 00
};

void setup() {
  clock_prescale_set(clock_div_2);
  Serial.begin(115200);
  while(!Serial);
  delay(500);
  Serial.println("Loading...");
  setDataDirection(INPUT);
  for (int i=0; i < NUMBER_OF_DATA_PINS; i++) {
    pinMode(DATA_PINS[i], OUTPUT);
    digitalWrite(DATA_PINS[i], LOW);
  }
  pinMode(PAUSE, OUTPUT);
  digitalWrite(PAUSE, LOW);
  pinMode(SHIFT_ENABLE, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);
  setAddress(0);
  digitalWrite(PAUSE, HIGH);
  digitalWrite(OUTPUT_ENABLE, LOW);
  digitalWrite(WRITE_ENABLE, LOW);
  pinMode(OUTPUT_ENABLE, OUTPUT);
  pinMode(WRITE_ENABLE, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

// Load the program
  for (long i = 0; i < CODE_SIZE; i++) {
    setAddress(i);
    setDataDirection(OUTPUT);
    setDataByte(CODE[i]);
    digitalWrite(WRITE_ENABLE, HIGH);
    delayMicroseconds(10);
    digitalWrite(WRITE_ENABLE, LOW);
  
  // Verify the program
    setDataDirection(INPUT);
    digitalWrite(OUTPUT_ENABLE, HIGH);
    delayMicroseconds(10);
    int v = getDataByte();
    if (v != CODE[i]) {
      Serial.print("Error at=");
      Serial.println(i);
      while(1) {
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(200);
      }
    }
    digitalWrite(OUTPUT_ENABLE, LOW);
  }
  setAddress(0);
  digitalWrite(OUTPUT_ENABLE, LOW);
  digitalWrite(WRITE_ENABLE, LOW);
  pinMode(OUTPUT_ENABLE, INPUT);
  pinMode(WRITE_ENABLE, INPUT);
  digitalWrite(PAUSE, LOW);
  Serial.println("Loaded...");
  return;
}

// Set the direction of the data pins
void setDataDirection(int direction) {
  for (int i = 0; i < NUMBER_OF_DATA_PINS; i++) {
    pinMode(DATA_PINS[i], direction);
  }
  return;
}

void setDataByte(int value) {
  value = value & 0x000000ff;
  for (int i = 0; i < NUMBER_OF_DATA_PINS; i++) {
    if ((value & DATA_MASKS[i]) == 0) {
      digitalWrite(DATA_PINS[i], LOW);
    } else {
      digitalWrite(DATA_PINS[i], HIGH);
    }
  }
  return;
}

int getDataByte() {
  int value = 0;
  for (int i = 0; i < NUMBER_OF_DATA_PINS; i++) {
    if (digitalRead(DATA_PINS[i]) == HIGH) {
      value += DATA_MASKS[i];
    }
  }
  return value;
}

void setAddress(long address) {
  int high = (address & 0x0000FF00) >> 8;
  int low = address & 0x000000FF;
  digitalWrite(SHIFT_ENABLE, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, high);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, low);
  digitalWrite(SHIFT_ENABLE, HIGH);
  return;
}


void loop() {
  while(1) {
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
  }
  return;
 }
