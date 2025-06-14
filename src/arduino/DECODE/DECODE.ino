#include "InstructionData.h"
#define NUMBER_OF_INSTRUCTION_PINS 8
#define NUMBER_OF_CLOCK_STEP_PINS 5

// Clock
#define STEP_CLR     0

// Program Counter
#define PCL_IN       1
#define PCL_SET      2
#define PCL_OUT      3
#define PCH_IN       4
#define PCH_SET      5
#define PCH_OUT      6
#define PC_IN        7
#define PC_SET       8
#define PC_OUT       9

// Memory Manager Unit
#define MEN_IN      10
#define MEN_OUT     11

// Instruction Register
#define IR_IN       12
#define IR_SET      13

// Interupted Control
#define IRQSUP_SET  14
#define IRQSUP_CLR  15
#define IRQWAIT_CLR 16

// Registers
#define REGA_IN     17
#define REGA_SET    18
#define REGA_OUT    19
#define REGB_IN     20
#define REGB_SET    21
#define REGB_OUT    22
#define REGS1_IN    23
#define REGS1_SET   24
#define REGS1_OUT   25
#define REGS2_IN    26
#define REGS2_SET   27
#define REGS2_OUT   28
#define REGC_IN     29 // Also does REGF_IN
#define REGC_SET    30 // Also does REGF_SET
#define REGC_OUT    31
#define REGF_OUT    32
#define CIN_SET     33
#define CIN_CLR     34

// Memory pointer
#define MPL_IN      35
#define MPL_SET     36
#define MPH_IN      37
#define MPH_SET     39
#define MP_OUT      40

// Stack Pointer
#define SPL_IN      41
#define SPL_SET     42
#define SPL_OUT     43
#define SPH_IN      44
#define SPH_SET     45
#define SPH_OUT     46
#define SP_OUT      47
#define SP_INC      48
#define SP_DEC      49

// Interupt Vector
#define IVL_IN      50
#define IVL_SET     51
#define IVL_OUT     52
#define IVH_IN      53
#define IVH_SET     54
#define IVH_OUT     55

// Special functions
#define RST         63


#define LATCH_PIN    3
#define CLOCK_PIN    2
#define DATA_PIN     4
#define LED_PIN     25

#define CARRY_PIN    5 
#define EQU_PIN     28
#define ZERO_PIN    27
#define NEG_PIN     26
#define IRQ_PIN     22
#define NC1_PIN     21
#define NC2_PIN     20
#define NC3_PIN     19

#define CARRY_MASK  0b00000000000000000000000000100000
#define EQU_MASK    0b00010000000000000000000000000000
#define ZERO_MASK   0b00001000000000000000000000000000
#define NEG_MASK    0b00000100000000000000000000000000
#define IRQ_MASK    0b00000000010000000000000000000000
#define NC1_MASK    0b00000000001000000000000000000000
#define NC2_MASK    0b00000000000100000000000000000000
#define NC3_MASK    0b00000000000010000000000000000000

#define CARRY_VALUE_MASK  0b00000001
#define EQU_VALUE_MASK    0b00000010
#define ZERO_VALUE_MASK   0b00000100
#define NEG_VALUE_MASK    0b00001000
#define IRQ_VALUE_MASK    0b00010000
#define NC1_VALUE_MASK    0b00100000
#define NC2_VALUE_MASK    0b01000000
#define NC3_VALUE_MASK    0b10000000

#define INSTRUCTION_PIN_0 6
#define INSTRUCTION_PIN_1 7
#define INSTRUCTION_PIN_2 8
#define INSTRUCTION_PIN_3 9
#define INSTRUCTION_PIN_4 10
#define INSTRUCTION_PIN_5 11
#define INSTRUCTION_PIN_6 12
#define INSTRUCTION_PIN_7 13

#define CLOCK_PIN_0 14
#define CLOCK_PIN_1 15
#define CLOCK_PIN_2 16
#define CLOCK_PIN_3 17
#define CLOCK_PIN_4 18

//const uint8_t instructions [256 * 32 * 256 * 5] = {};

const int INSTRUCTION_PINS[NUMBER_OF_INSTRUCTION_PINS] = { INSTRUCTION_PIN_0, INSTRUCTION_PIN_1, INSTRUCTION_PIN_2,
 INSTRUCTION_PIN_3, INSTRUCTION_PIN_4, INSTRUCTION_PIN_5, INSTRUCTION_PIN_6, INSTRUCTION_PIN_7 };
const int INSTRUCTION_VALUES[NUMBER_OF_INSTRUCTION_PINS] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };
const uint32_t INSTRUCTION_MASKS[NUMBER_OF_INSTRUCTION_PINS] = { 
  0b00000001000000,
  0b00000010000000,
  0b00000100000000,
  0b00001000000000,
  0b00010000000000,
  0b00100000000000,
  0b01000000000000,
  0b10000000000000 };
const int CLOCK_STEP_PINS[NUMBER_OF_CLOCK_STEP_PINS] = { CLOCK_PIN_0, CLOCK_PIN_1, CLOCK_PIN_2, CLOCK_PIN_3, CLOCK_PIN_4 };
const int CLOCK_STEP_VALUES[NUMBER_OF_CLOCK_STEP_PINS] = { 0b00001, 0b00010, 0b00100, 0b01000, 0b10000 };
const uint32_t CLOCK_STEP_MASKS[NUMBER_OF_CLOCK_STEP_PINS] = {
  0b000010000000000000,
  0b000100000000000000,
  0b001000000000000000,
  0b010000000000000000,
  0b100000000000000000 };

uint8_t instruction = 0;
uint8_t clockStep = 0; 
uint8_t lastInstruction = 0;
uint8_t lastClockStep = 0;
bool carryFlag = false;
bool equalsFlag = false;
bool zeroFlag = false;
bool negativeFlag = false;
bool interruptFlag = false;
bool nc1Flag = false;
bool nc2Flag = false;
bool nc3Flag = false;
uint8_t flags = 0;
uint8_t lastFlags = 0;
int ledCounter = 0;
bool ledOn = false;


void readInput() {
  int j = 0;
  uint32_t pins = 0;
  uint32_t lastPins = 0;
  while(true) {
    pins = gpio_get_all();
    if (pins == lastPins) {
      j++;
      if (j > 4) {
        break;
      }
    } else {
      lastPins = pins;
      j = 0;
    }
    delayMicroseconds(10);
  }

  instruction = 0;
  for (int i = 0; i < NUMBER_OF_INSTRUCTION_PINS; i++) {
    if ((pins & INSTRUCTION_MASKS[i]) != 0) {
      instruction += INSTRUCTION_VALUES[i];
    }
  }

  clockStep = 0;
  for (int i = 0; i < NUMBER_OF_CLOCK_STEP_PINS; i++) {
    if ((pins & CLOCK_STEP_MASKS[i]) != 0) {
      clockStep += CLOCK_STEP_VALUES[i];
    }
  }

  bool carryFlag = false;
  bool equalsFlag = false;
  bool zeroFlag = false;
  bool negativeFlag = false;
  bool interruptFlag = false;
  bool nc1Flag = false;
  bool nc2Flag = false;
  bool nc3Flag = false;
  flags = 0;
  if ((pins & CARRY_MASK) != 0) {
    flags += CARRY_VALUE_MASK;
    carryFlag = true;
  }
  if ((pins & EQU_MASK) != 0) {
    flags += EQU_VALUE_MASK;
    equalsFlag = true;
  }
  if ((pins & ZERO_MASK) != 0) {
    flags += ZERO_VALUE_MASK;
    zeroFlag = true;
  }
  if ((pins & NEG_MASK) != 0) {
    flags += NEG_VALUE_MASK;
    negativeFlag = true;
  }
  if (pins & IRQ_MASK) {
    flags += IRQ_VALUE_MASK;
    interruptFlag = true;
  }
  if (pins & NC1_MASK) {
    flags += NC1_VALUE_MASK;
    nc1Flag = true;
  }
  if (pins & NC2_MASK) {
    flags += NC2_VALUE_MASK;
    nc2Flag = true;
  } 
    if (pins & NC3_MASK) {
    flags += NC3_VALUE_MASK;
    nc3Flag = true;
  } 
}

void setup() {
  // Set up serial port
  Serial.begin(115200);
  Serial.setTimeout(20);

  // set pins to control the shift register
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Set up the instruction pins
  for (int i = 0; i < NUMBER_OF_INSTRUCTION_PINS; i++) {
    pinMode(INSTRUCTION_PINS[i], INPUT);
  }

  // Set up the clock pins
  for (int i = 0; i < NUMBER_OF_CLOCK_STEP_PINS; i++) {
    pinMode(CLOCK_STEP_PINS[i], INPUT);
  }

  // And the other pins
  pinMode(CARRY_PIN, INPUT);
  pinMode(EQU_PIN, INPUT);
  pinMode(ZERO_PIN, INPUT);
  pinMode(NEG_PIN, INPUT);
  pinMode(IRQ_PIN, INPUT);
  pinMode(NC1_PIN, INPUT);
  pinMode(NC2_PIN, INPUT);
  pinMode(NC3_PIN, INPUT);

// Give everything a chance to get ready
  Serial.println("Starting...");
  delay(100);
}

void loop() {
  readInput();

  if (lastInstruction != instruction || lastClockStep != clockStep || lastFlags != flags) {
    Serial.print(instruction, HEX);
    Serial.print(" ");
    Serial.print(clockStep, HEX);
    Serial.print(" ");
    Serial.print(flags, HEX);
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, instruction);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, clockStep);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, flags);
    digitalWrite(LATCH_PIN, HIGH);
    lastInstruction = instruction;
    lastClockStep = clockStep;
    lastFlags = flags;
  }
  ledCounter += 1;
  if (ledCounter > 5000) {
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn);
    ledCounter = 0;
  } 
}