#include <avr/power.h>
#include <U8g2lib.h>

#define NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN 5
#define NUMBER_OF_TIMES_TO_CHECK_IRQ_BUTTON_PIN 20
#define NUMBER_OF_TIMES_TO_CHECK_RST_BUTTON_PIN 20
#define NUMBER_OF_TIMES_TO_CHECK_CLK_CLR_PIN 2
#define BUTTON_PRESSED_ABOVE 512
#define SHIFT_ENABLE_PIN    A0
#define SHIFT_CLK_PIN       A1
#define SHIFT_DATA_PIN      A2
#define MODE_PIN 5
#define UP_PIN 6
#define DOWN_PIN 13
#define STEP_PIN A7
#define IRQ_PIN A6
#define RST_PIN 11
#define IRQWAIT_CLR_PIN 10
#define IRQSUP_CLR_PIN 9
#define IRQSUP_SET_PIN 8
#define CLK_CLR_PIN 7

#define IRQWAIT_OUT_PIN 3
#define RST_OUT_PIN A3
#define IRQ_OUT_PIN 4
#define IRQSUP_OUT_PIN 2

#define MODE_PIN_MASK 0b00100000
#define CLK_CLR_PIN_MASK 0b10000000
#define IRQSUP_SET_PIN_MASK 0b00000001
#define IRQSUP_CLR_PIN_MASK 0b00000010

#define MODE_NULL 0
#define MODE_RUN 1
#define MODE_STEP 2

#define NUMBER_OF_CLK_PINS 5

uint32_t lastCheckModeButton = 0;
uint32_t lastCheckUpButton = 0;
uint32_t lastCheckDownButton = 0;
uint32_t lastCheckStepButton = 0;
uint8_t modeButtonCounter = 0;
uint8_t modeButtonPressedCounter = 0;
uint8_t upButtonCounter = 0;
uint8_t upButtonPressedCounter = 0;
uint8_t downButtonCounter = 0;
uint8_t downButtonPressedCounter = 0;
uint8_t stepButtonCounter = 0;
uint8_t irqButtonCounter = 0;
uint8_t rstButtonCounter = 0;
bool modeButtonPressed = false;
bool upButtonPressed = false;
bool downButtonPressed = false;
bool stepButtonPressed = false;
bool irqButtonPressed = false;
bool rstButtonPressed = false;
bool irqSupSetPressed = false;
bool irqSupClrPressed = false;
bool irqWaitClrPressed = false;
bool clkClrPressed = false;
bool irq = false;
bool irqWait = false;
bool irqSup = false;
bool clkClr = false;
bool resetFlag = false;
int mode = MODE_STEP;
int lastMode = MODE_NULL;
int clkSpeed = 0;
int lastClkSpeed = -1;
int clkInc = -1;
int irqWaitClrCounter = 0;
uint32_t lastClkInc = 0;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

void updateClk() {
  checkClkClr();
  clkInc++;
  if (clkInc > 31 || (clkClr && clkInc > 2)) { // The min step count is 3
    clkInc = 0;
    clkClr = false;
    if (irqWait && !irqSup) {
      digitalWrite(IRQSUP_OUT_PIN, HIGH);
      digitalWrite(IRQ_OUT_PIN, HIGH);
      digitalWrite(IRQWAIT_OUT_PIN, LOW);
      irqWait = false;
      irq = true;
      irqSup = true;
    } else {
      if (irq) {
        digitalWrite(IRQ_OUT_PIN, LOW);
        irq = false;
      }
    }
  }
  // Set all the clock pins at the same time.
  uint8_t value = clkInc & 0b00011111;
  digitalWrite(SHIFT_ENABLE_PIN, LOW);
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, value);
  digitalWrite(SHIFT_ENABLE_PIN, HIGH);
  Serial.println(clkInc);
  lastClkInc = millis();
  return;
}

void updateScreen() {
  if (lastMode != mode || lastClkSpeed != clkSpeed) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Mode:");
    switch(mode) {
      case MODE_RUN:
        u8g2.drawStr(40, 10, "RUN");
        break;
      case MODE_STEP:
        u8g2.drawStr(40, 10, "STEP");
        break;
      default:
        break;
    }
    u8g2.drawStr(0, 25, "Speed:");
    u8g2.drawStr(40, 25, String(clkSpeed).c_str());
    u8g2.sendBuffer();
    lastMode = mode;
    lastClkSpeed = clkSpeed;
  }
}

void reset() {
  digitalWrite(RST_OUT_PIN, HIGH);
  Serial.println("Reset...");
  delay(200);
  digitalWrite(RST_OUT_PIN, LOW);
  digitalWrite(IRQWAIT_OUT_PIN, LOW);
  digitalWrite(IRQSUP_OUT_PIN, HIGH);
  digitalWrite(IRQ_OUT_PIN, LOW);
  digitalWrite(RST_OUT_PIN, LOW);
  lastCheckModeButton = 0;
  lastCheckUpButton = 0;
  lastCheckDownButton = 0;
  lastCheckStepButton = 0;
  modeButtonCounter = 0;
  modeButtonPressedCounter = 0;
  upButtonCounter = 0;
  upButtonPressedCounter = 0;
  downButtonCounter = 0;
  downButtonPressedCounter = 0;
  stepButtonCounter = 0;
  irqButtonCounter = 0;
  rstButtonCounter = 0;
  irq = false;
  irqWait = false;
  irqSup = true;
  clkClr = false;
  clkInc = -1;
  updateClk();
}

void setup() {
  clock_prescale_set(clock_div_2); // Clock running at 32Mhz. Comment out if standard 16Mhz.
  Serial.begin(115200);
  while(!Serial);
  delay(500);
  Serial.println("Starting...");

  u8g2.begin();

  pinMode(SHIFT_ENABLE_PIN, OUTPUT);
  pinMode(SHIFT_CLK_PIN, OUTPUT);
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  digitalWrite(SHIFT_ENABLE_PIN, LOW);
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, clkInc);
  digitalWrite(SHIFT_ENABLE_PIN, HIGH);

  // Set up the other pins
  pinMode(RST_OUT_PIN, OUTPUT);
  pinMode(IRQWAIT_OUT_PIN, OUTPUT);
  pinMode(IRQ_OUT_PIN, OUTPUT);
  pinMode(IRQSUP_OUT_PIN, OUTPUT);
  digitalWrite(RST_OUT_PIN, LOW);
  digitalWrite(IRQWAIT_OUT_PIN, LOW);
  digitalWrite(IRQ_OUT_PIN, LOW);
  digitalWrite(IRQSUP_OUT_PIN, LOW);

  // Set up the buttons
  pinMode(MODE_PIN, INPUT);
  pinMode(UP_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(IRQSUP_CLR_PIN, INPUT);
  pinMode(CLK_CLR_PIN, INPUT);

  // Reset the system
  delay(500);
  reset();
}


void checkModeButton() {
  if (PIND & MODE_PIN_MASK) {
    if (!modeButtonPressed) {
      modeButtonCounter++;
      if (modeButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
        if (mode == MODE_RUN) {
          mode = MODE_STEP;
        } else {
          mode = MODE_RUN;
        }
        modeButtonPressed = true;
        modeButtonCounter = 0;
      }
    }
  } else {
    if (modeButtonPressed) {
      modeButtonPressed = false;
      modeButtonCounter = 0;
    }
  }
  lastCheckModeButton = millis();
}

void checkUpButton() {
  if (digitalRead(UP_PIN)) {
    if (!upButtonPressed) {
      upButtonCounter++;
      if (upButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
        upButtonPressed = true;
        upButtonCounter = 0;
        clkSpeed = clkSpeed + 50;
        if (clkSpeed > 1000) {
          clkSpeed = 1000;
        }
      }
    } else {
      upButtonPressedCounter++;
      if (upButtonPressedCounter > 50) {
        clkSpeed = clkSpeed + 50;
        if (clkSpeed > 1000) {
          clkSpeed = 1000;
        }
        upButtonPressedCounter = 0;
      }
    }
  } else {
    if (upButtonPressed) {
      upButtonCounter++;
      if (upButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
        upButtonPressed = false;
        upButtonCounter = 0;
        upButtonPressedCounter = 0;
      }
    }
  }
  lastCheckUpButton = millis();
}

void checkDownButton() {
  if (digitalRead(DOWN_PIN)) {
    if (!downButtonPressed) {
      downButtonCounter++;
      if (downButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
        downButtonPressed = true;
        downButtonCounter = 0;
        clkSpeed = clkSpeed - 50;
        if (clkSpeed < 0) {
          clkSpeed = 0;
        }
      }
    } else {
      downButtonPressedCounter++;
      if (downButtonPressedCounter > 50) {
        clkSpeed = clkSpeed - 50;
        if (clkSpeed < 0) {
          clkSpeed = 0;
        }
        downButtonPressedCounter = 0;
      }
    }
   } else {
    if (downButtonPressed) {
      downButtonCounter++;
      if (downButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
        downButtonPressed = false;
        downButtonCounter = 0;
        downButtonPressedCounter = 0;
      }
    }
   }
   lastCheckDownButton = millis();
}

void checkStepButton() {
  if (mode == MODE_STEP) {
    if (analogRead(STEP_PIN) > BUTTON_PRESSED_ABOVE) {
      if (!stepButtonPressed) {
        stepButtonCounter++;
        if (stepButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
          updateClk();
          stepButtonPressed = true;
          stepButtonCounter = 0;
        }
      }
    } else {
     if (stepButtonPressed) {
       stepButtonCounter++;
       if (stepButtonCounter > NUMBER_OF_TIMES_TO_CHECK_BUTTON_PIN) {
         stepButtonPressed = false;
         stepButtonCounter = 0;
       }
      }
    }
  }
   lastCheckStepButton = millis();
}

void checkIrqButton() {
  if (!irqWait) {
    if (analogRead(IRQ_PIN) > BUTTON_PRESSED_ABOVE) {
      if (!irqButtonPressed) {
        irqButtonCounter++;
        if (irqButtonCounter > NUMBER_OF_TIMES_TO_CHECK_IRQ_BUTTON_PIN) {
         irqButtonPressed = true;
         irqButtonCounter = 0;
         irqWait = true;
         digitalWrite(IRQWAIT_OUT_PIN, HIGH);
        }
      }
    }
  } else {
    if (irqButtonPressed) {
      irqButtonCounter++;
      if (irqButtonCounter > NUMBER_OF_TIMES_TO_CHECK_IRQ_BUTTON_PIN) {
        irqButtonPressed = false;
        irqButtonCounter = 0;
      }
    }
  }
}

void checkRstButton() {
  if (digitalRead(RST_PIN)) {
    if (!rstButtonPressed) {
      rstButtonCounter++;
      if (rstButtonCounter > NUMBER_OF_TIMES_TO_CHECK_RST_BUTTON_PIN) {
        rstButtonPressed = true;
        rstButtonCounter = 0;
        digitalWrite(RST_OUT_PIN, HIGH);
        reset();
      }
    }
  } else {
    if (rstButtonPressed) {
      rstButtonCounter++;
      if (rstButtonCounter > NUMBER_OF_TIMES_TO_CHECK_RST_BUTTON_PIN) {
        rstButtonPressed = false;
        rstButtonCounter = 0;
        digitalWrite(RST_OUT_PIN, LOW);
      }
    }
  }
}

void checkIrqSupSet() {
  if (PINC & IRQSUP_SET_PIN_MASK) {
    if (!irqSupSetPressed) {
      irqSup = true;
      irqSupSetPressed = true;
      digitalWrite(IRQSUP_OUT_PIN, HIGH);
    }
  } else {
    if (irqSupSetPressed) {
      irqSupSetPressed = false;
    }
  }
}

void checkIrqSupClr() {
  if (PINC & IRQSUP_CLR_PIN_MASK) {
    if (!irqSupClrPressed) {
      irqSup = false;
      irqSupClrPressed = true;
      digitalWrite(IRQSUP_OUT_PIN, LOW);
    }
  } else {
    if (irqSupClrPressed) {
      irqSupClrPressed = false;
    }
  }
}

void checkClkClr() {
  if (PIND & CLK_CLR_PIN_MASK) {
    if (!clkClrPressed) {
      clkClrPressed = true;
      clkClr = true;
    }
  } else {
    if (clkClrPressed) {
      clkClrPressed = false;
    }
  }
}


void loop() {
  if (irqSup) {
    checkIrqSupClr();
  } else {
    checkIrqSupSet();
  }
  if (millis() - lastCheckModeButton >= 10) {
    checkModeButton();
  }
  if (millis() - lastCheckUpButton >= 10) {
    checkUpButton();
  }
  if (millis() - lastCheckDownButton >= 10) {
    checkDownButton();
  }
  if (millis() - lastCheckStepButton >= 10) {
    checkStepButton();
  }
  checkIrqButton();
  checkRstButton();
  if (mode == MODE_RUN) {
    if (millis() - lastClkInc >= (1000 - clkSpeed) + 1) {
      updateClk();
    }
  }
  updateScreen();
}


