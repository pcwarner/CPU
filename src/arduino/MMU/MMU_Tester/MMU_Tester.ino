// Used to set clock prescaler
#include <avr/power.h>

#define UNKNOWN_DIRECTION 0xff
#define MAX_COMMAND_LINE_LENGTH 20
#define MAX_GIVE_UP 20
#define ADDRESS_ARRAY_SIZE 4
#define DATA_ARRAY_SIZE 2
#define EPROM_ARRAY_SIZE 2
#define MAX_EPROM_NAME_SIZE 6
#define MAX_NUMBER_OF_PARTS 4
#define MAX_SIZE_OF_A_PART 8




// Define pins used for the shift register
const int SHIFT_ENABLE = A0;  // Move buffer to register pin
const int SHIFT_CLK = A1;     // Shift register clock pin
const int SHIFT_DATA = A2;    // Shift register data pin

// Define pins used to control access to UVEPROM
const int WRITE_ENABLE = A3;
const int OUTPUT_ENABLE = 4; // Use A4 in later boards 
const int PAUSE = A5; 

const int DATA_BIT_0 = 5;   // Data bit 0 pin
const int DATA_BIT_1 = 6;   // Data bit 1 pin
const int DATA_BIT_2 = 7;   // Data bit 2 pin
const int DATA_BIT_3 = 8;   // Data bit 3 pin
const int DATA_BIT_4 = 9;   // Data bit 4 pin
const int DATA_BIT_5 = 10;  // Data bit 5 pin
const int DATA_BIT_6 = 11;  // Data bit 6 pin
const int DATA_BIT_7 = 12;  // Data bit 7 pin

const int DATA_PIN_ARRAY[8] = { DATA_BIT_0, DATA_BIT_1, DATA_BIT_2, DATA_BIT_3, DATA_BIT_4, DATA_BIT_5, DATA_BIT_6, DATA_BIT_7 };
const int DATA_MASK_ARRAY[8] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };


#define ADDRESS_BIT_0 0x1
#define ADDRESS_BIT_1 0x2
#define ADDRESS_BIT_2 0x4
#define ADDRESS_BIT_3 0x8
#define ADDRESS_BIT_4 0x10
#define ADDRESS_BIT_5 0x20
#define ADDRESS_BIT_6 0x40
#define ADDRESS_BIT_7 0x80
#define ADDRESS_BIT_8 0x100
#define ADDRESS_BIT_9 0x200
#define ADDRESS_BIT_10 0x400
#define ADDRESS_BIT_11 0x800
#define ADDRESS_BIT_12 0x1000
#define ADDRESS_BIT_13 0x2000
#define ADDRESS_BIT_14 0x4000
#define ADDRESS_BIT_15 0x8000

// Global variables
int dataDirection = 9999;

// Check pause
void checkPause() {
  while(digitalRead(PAUSE) > 0) {
    Serial.println("Pause...");
    delay(1000);
  }
}

// Set the address we need to read from or write to
void setAddress(long address) {
  address = address & 0x0000ffff;
  byte low = address % 0x0100;
  byte high = (address - low) / 0x0100;
  digitalWrite(SHIFT_ENABLE, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, high);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, low);
  digitalWrite(SHIFT_ENABLE, HIGH);
  return;
}

// Set the direction of the data pins
void setDataDirection(int direction) {
  if (dataDirection != direction) {
    for (int i = 0; i < 8; i++) {
      pinMode(DATA_PIN_ARRAY[i],  direction);
    }
    dataDirection = direction;
  }
  return;
}

void setDataByte(int value) {
  value = value & 0x000000ff;
  for (int i = 0; i < 8; i++) {
    if ((value & DATA_MASK_ARRAY[i]) == 0) {
      digitalWrite(DATA_PIN_ARRAY[i], LOW);
    } else {
      digitalWrite(DATA_PIN_ARRAY[i], HIGH);
    }
  }
  return;
}

int getDataByte() {
  int value = 0;
  for (int i = 0; i < 8; i++) {
    value += (digitalRead(DATA_PIN_ARRAY[i]) * DATA_MASK_ARRAY[i]);
  }
  return value;
}

int readByte(long address) {
  checkPause();
  setAddress(address);
  setDataDirection(INPUT);
  digitalWrite(OUTPUT_ENABLE, HIGH);
  delayMicroseconds(1);
  int value = getDataByte();
  digitalWrite(OUTPUT_ENABLE, LOW);
  return value;
}

void writeByte(long address, int value) {
  checkPause();
  setAddress(address); 
  setDataDirection(OUTPUT);
  setDataByte(value);
  digitalWrite(WRITE_ENABLE, HIGH);
  delayMicroseconds(1); 
  digitalWrite(WRITE_ENABLE, LOW);
  return;
}



void setup() {
  // Set up the serial link
  clock_prescale_set(clock_div_2);
  Serial.begin(115200);
  Serial.setTimeout(20);

  // Set up the pins
  pinMode(SHIFT_ENABLE, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(OUTPUT_ENABLE, OUTPUT);
  pinMode(WRITE_ENABLE, OUTPUT);
  pinMode(PAUSE, INPUT);

  digitalWrite(OUTPUT_ENABLE, LOW);
  digitalWrite(WRITE_ENABLE, LOW);


  // Set up the data control pins
  dataDirection = UNKNOWN_DIRECTION;
  setDataDirection(INPUT);
  setAddress(0);

  // Display message
  Serial.println("Starting...");
  return;
}

void loop() {
  for (long i=0; i<0xff00; i++) {
    writeByte(i, i & 0x00ff);
  }
  for (long i=0; i<0xff00; i++) {
    int j = readByte(i);
    if (i != i & 0x00ff) {
      Serial.print(i);
    }
  }
  Serial.println("Memory test done...");
  delay(1000);
  Serial.println(readByte(0xff01));
  Serial.println("Done...");
  while(1) {}
  return;
}