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


#define EPROM_UNKNOWN 0
#define EPROM_27C16 1
#define EPROM_27C256 2

const char EPROM_NAMES[EPROM_ARRAY_SIZE][MAX_EPROM_NAME_SIZE + 1] = { "27C16", "27C256" };
const int EPROM_TYPES[EPROM_ARRAY_SIZE] = { EPROM_27C16, EPROM_27C256 };


// Define pins used for the shift register
const int SHIFT_ENABLE = A0;  // Move buffer to register pin
const int SHIFT_CLK = A1;     // Shift register clock pin
const int SHIFT_DATA = A2;    // Shift register data pin

// Define pins used to control access to UVEPROM
const int OUTPUT_ENABLE = A3;
const int CHIP_ENABLE = 4; // Use A4 in later boards 
const int VPP_ENABLE = A5; 

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

const int ENABLE = LOW;    // 0v is enable - These are the reverse to what you would expect
const int DISABLE = HIGH;  // 5v is disable

#define READ_COMMAND 'R'
#define WRITE_COMMAND 'W'
#define UPDATE_COMMAND 'U'
#define SET_COMMAND 'S'
#define GET_COMMAND 'G'
#define BATCH_COMMAND 'B'
#define HELP_COMMAND 'H'
#define TEST_COMMAND 'T'
#define END_OF_LINE 10

#define MAIN_METHOD 0
#define READ_METHOD 1
#define WRITE_METHOD 2
#define UPDATE_METHOD 3
#define PROTECT_METHOD 4
#define SET_METHOD 5
#define GET_METHOD 6
#define BATCH_METHOD 7
#define HELP_METHOD 8
#define TEST_METHOD 9

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
int epromType = EPROM_UNKNOWN;
bool vppBatchModeEnabled = false;

int split(char input[], char splitChar, char output[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  int j = 0;
  int k = -1;
  int l = strlen(input);

  for (int i = 0; i < l; i++) {
    char c = input[i];
    if (c == splitChar) {
      output[j][k + 1] = 0;
      j++;
      if (j > MAX_NUMBER_OF_PARTS - 1) {
        return -1;
      }
      k = -1;
    } else {
      k++;
      if (k > MAX_SIZE_OF_A_PART - 1) {
        return -1;
      }
      output[j][k] = c;
      output[j][k + 1] = 0;
    }
  }
  return j + 1;
}

int char2HEX(char digit) {
  int value = int(toupper(digit));
  if (value >= 48 && value <= 57) {  // Numbers
    return value - (int)'0';
  }
  if (value >= 65 && value <= 70) {  // Letters
    return 10 + value - (int)'A';
  }
  return -1;
}


long parseHEX(char value[]) {
  long result = 0;
  for (int i = 0; i < strlen(value); i++) {
    result *= 0x10;
    int c = char2HEX(value[i]);
    if (c < 0) {
      return -1;
    }
    result += c;
  }
  return result;
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

int readByte(long address, bool *error) {
  setAddress(address);
  setDataDirection(INPUT);
  digitalWrite(CHIP_ENABLE, HIGH);
  int value = getDataByte();
  digitalWrite(CHIP_ENABLE, LOW);
  *error = false;
  return value;
}

void writeByte(long address, int value, bool *error) {
  setAddress(address);
  setDataDirection(OUTPUT);
  setDataByte(value);
  digitalWrite(OUTPUT_ENABLE, HIGH);
  digitalWrite(OUTPUT_ENABLE, LOW);
  *error = false;
  return;
}

void displayError(int errorFrom, int errorNumber) {
  Serial.print(F("E,"));
  switch(errorFrom) {
    case READ_METHOD:
      Serial.print(F("READ"));
      break;
    case WRITE_METHOD:
      Serial.print(F("WRITE"));
      break;
    case UPDATE_METHOD:
      Serial.print(F("UPDATE"));
      break;
    case PROTECT_METHOD:
      Serial.print(F("PROTECT"));
      break;
    case SET_METHOD:
      Serial.print(F("SET"));
      break;
    case GET_METHOD:
      Serial.print(F("GET"));
      break;
    case HELP_METHOD:
      Serial.print(F("HELP"));
      break;
    case TEST_METHOD:
      Serial.print(F("TEST"));
      break;
    case MAIN_METHOD:
      Serial.print(F("MAIN"));
      break;
    default:
      Serial.print(F("OTHER"));
      break;
  }
  
  Serial.print(F(": "));
  
  switch(errorNumber) {
    case 1:
      Serial.println(F("EPROM type not set."));
      break;
    case 2:
      Serial.println(F("The syntax of the command is incorrect"));
      break;
    case 3:
      Serial.println(F("Address is not a valid HEX number."));
      break;
    case 4:
      Serial.println(F("Data is not a valid HEX number."));
      break;
    case 5:
      Serial.println(F("Data written to EPROM did not match that read."));
      break;
    case 6:
      Serial.println(F("EPROM name is invalid."));
      break;
    case 7:
      Serial.println(F("EPROM type is not valid."));
      break;
    case 8:
      Serial.println(F("Failed to find line termination."));
      break;
    case 9:
      Serial.println(F("Data in buffer present when not expected."));
      break;
    case 10:
      Serial.println(F("Unknown command or incorrect syntax."));
      break;
    case 11:
      Serial.println(F("Protect must be followed by 0 (Unprotect) or 1 (Protect)."));
      break;
    case 12:
      Serial.println(F("Value is not a valid number."));
      break;
    default:
      Serial.println(F("Unknown error."));
      break;
  }  
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
  pinMode(CHIP_ENABLE, OUTPUT);
  pinMode(VPP_ENABLE, OUTPUT);

  digitalWrite(OUTPUT_ENABLE, LOW);
  digitalWrite(CHIP_ENABLE, LOW);
  digitalWrite(VPP_ENABLE, LOW);


  // Set up the data control pins
  dataDirection = UNKNOWN_DIRECTION;
  setDataDirection(INPUT);
  setAddress(0);

  // Clear out anything in the shift register
  Serial.println("G");
  delay(500);

  return;
}

void read(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {

  if (numberOfParts != 2) {
    displayError(READ_METHOD, 2);
    return;
  }


  // Convert address (in the second part) to a long
  long address = parseHEX(commandLineParts[1]);
  if (address < 0) {
    displayError(READ_METHOD, 3);
    return;
  }

  bool error = false;
  int data = readByte(address, &error);
  if (!error) {
    Serial.print("G,");
    Serial.println(data, HEX);
  }
  return;
}

void write(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {

  if (numberOfParts != 3) {
    displayError(WRITE_METHOD, 2);
    return;
  }

  // Convert address to a long
  long address = parseHEX(commandLineParts[1]);
  if (address < 0) {
    displayError(WRITE_METHOD, 3);
    return;
  }

  // Convert data to a int
  int data = parseHEX(commandLineParts[2]);
  if (data < 0) {
    displayError(WRITE_METHOD, 4);
    return;
  }

  bool error = false;
  writeByte(address, data, &error);
  if (error) {
    return;
  }
  Serial.println("G");
  return;
}



void set(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  // There should be 2 parts
  if (numberOfParts != 2) {
    displayError(SET_METHOD, 2);
    return;
  }

  // The EPROM name must be between 1 and 8 characters
  int l = strlen(commandLineParts[1]);
  if (l < 1 || l > 8) {
    displayError(SET_METHOD, 6);
    return;
  }

  // Upper case the characters
  for (int i = 0; i < l; i++) {
    commandLineParts[1][i] = toupper(commandLineParts[1][i]);
  }

  // Find the EPROM from list
  epromType = EPROM_UNKNOWN;
  for (int i = 0; i < EPROM_ARRAY_SIZE; i++) {
    if (strcmp(commandLineParts[1], EPROM_NAMES[i]) == 0) {
      epromType = EPROM_TYPES[i];
      break;
    }
  }
  if (epromType == EPROM_UNKNOWN) {
    displayError(SET_METHOD, 7);
    return;
  }

  Serial.println("G");
  return;
}


void loop() {
  if (Serial.available()) {
    int c = 0;
    char commandLine[MAX_COMMAND_LINE_LENGTH + 1];
    int pos = 0;
    int giveupCount = 0;
    char bytesRead[MAX_COMMAND_LINE_LENGTH + 1];
    do {
      int available = Serial.available();
      if (available > MAX_COMMAND_LINE_LENGTH) {
        available = MAX_COMMAND_LINE_LENGTH;
      }
      if (available > 0) {
        int numRead = Serial.readBytes(bytesRead, available);
        if (numRead > 0) {
          bytesRead[numRead] = 0;
          for (int j = 0; j < numRead; j++) {
            c = bytesRead[j];
            if (c != END_OF_LINE && pos < MAX_COMMAND_LINE_LENGTH - 1) {
              commandLine[pos] = (char)c;
              commandLine[pos + 1] = 0;
              pos++;
            } else {
              break;
            }
          }
        }
      } else {
        giveupCount++;
        delay(10);
      }
    } while (c != END_OF_LINE && pos < MAX_COMMAND_LINE_LENGTH && giveupCount < MAX_GIVE_UP);

    if (giveupCount >= MAX_GIVE_UP) {
      displayError(MAIN_METHOD, 8);
      return;
    }

    // Clear anything left in buffer
    bool bufferNotEmpty = false;
    while (Serial.available()) {
      Serial.read();
      bufferNotEmpty = true;
    }
    if (bufferNotEmpty) {
      displayError(MAIN_METHOD, 9);
      return;
    }

    // Split the command line
    char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1];
    int numberOfParts = split(commandLine, ',', commandLineParts);
    if (numberOfParts < 1 || strlen(commandLineParts[0]) < 1) {
      displayError(MAIN_METHOD, 2);
      return;
    }

    // Which command do we need to run?
    char command = toupper(commandLineParts[0][0]);
    switch (command) {
      case READ_COMMAND:
        read(numberOfParts, commandLineParts);
        break;
      case WRITE_COMMAND:
        write(numberOfParts, commandLineParts);
        break;
      case TEST_COMMAND:
        // digitalWrite(CHIP_ENABLE, HIGH);
        // delay(1000);
        // digitalWrite(CHIP_ENABLE, LOW);
        setDataDirection(OUTPUT);
      //  for(int i=0; i<10; i++) {
          setAddress(0b0000000001);
          delay(1000);
          setAddress(0b0000000000);

          setDataByte(0x55);
          setAddress(0b0100000000);
          setAddress(0b0110000000);

          setDataByte(0xAA);
          setAddress(0b0000100000);
          setAddress(0b0000110000);

          setDataByte(0);

          setAddress(0b0000000100);
          setAddress(0b0000000110);
          setAddress(0b0000001000);
          delay(5000);
          setAddress(0);

          setDataDirection(INPUT);
          setAddress(0b1000000000);
          int value = getDataByte();
          if (value == 0x55) {
            Serial.println("High value correct");
          } else {
            Serial.print("Error, value not as expected 0x55 was ");
            Serial.println(value, HEX);
            break;
          }

          setAddress(0b0001000000);
          value = getDataByte();
          if (value == 0xAA) {
            Serial.println("Low value correct");
          } else {
            Serial.print("Error, value not as expected 0xAA was ");
            Serial.println(value, HEX);
            break;
          }


          for(int i=0; i<255; i++) {
            setAddress(0b000000010);
            setAddress(0);
            delay(100);
          }
          setAddress(0b0000001000);
          delay(5000);
          
          // setDataByte(i * 11);
          // setAddress(0b100000);
          // setAddress(0b110000);
          // setAddress(0);
          // setDataByte(i * 13);
          // setAddress(0b001000);
          // setAddress(0b001100);
          // setAddress(0b000010);
          // delay(500);
          setAddress(0);
       // }
        Serial.println("G");
        break;
      default:
        displayError(MAIN_METHOD, 10);
        break;
    }
  }
  return;
}