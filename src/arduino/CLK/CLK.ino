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
#define STEP_PIN 4
#define IRQ_PIN 3
#define RST_PIN 11
#define IRQWAIT_CLR_PIN 10
#define IRQSUP_CLR_PIN 9
#define IRQSUP_SET_PIN 8
#define CLK_CLR_PIN 7
#define PAUSE_PIN 12
#define HALT_PIN 2

#define MODE_NULL 0
#define MODE_RUN 1
#define MODE_STEP 2

#define NUMBER_OF_CLK_PINS 8

#define MAX_CLOCK_DELAYS 10
const int ClockDelay[MAX_CLOCK_DELAYS+1] = {1000, 500, 250, 125, 62, 31, 15, 8, 4, 2, 1};

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
bool rst = false;
bool haltFlag = false;
bool pauseFlag = false;
bool lastPausedFlag = true;
bool lastHaltFlag = true;
int mode = MODE_STEP;
int lastMode = MODE_NULL;
int clkSpeed = 0;
int lastClkSpeed = -1;
int clkInc = 0;
int irqWaitClrCounter = 0;
uint32_t lastClkInc = 0;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

void updateClk() {
  checkClkClr();
  clkInc++;
  if (clkInc > 255 || (clkClr && clkInc > 2)) { // The min step count is 3
    clkInc = 0;
    clkClr = false;
    if (irqWait && !irqSup) {
      irqWait = false;
      irq = true;
      irqSup = true;
    } else {
      if (irq) {
        irq = false;
      }
    }
  }
  updateShift();
}

void updateShift() {
  // Set the flag and clock
  uint8_t high = 0;
  if (rst)     high = high + 0b00001000;
  if (irq)     high = high + 0b00000100;
  if (irqWait) high = high + 0b00000010;
  if (irqSup)  high = high + 0b00000001;
  uint8_t low = clkInc & 0b11111111;
  digitalWrite(SHIFT_ENABLE_PIN, LOW);
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, high);
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, low);
  digitalWrite(SHIFT_ENABLE_PIN, HIGH);
  lastClkInc = millis();
  return;
}

void updateScreen() {
  if (lastMode != mode || lastClkSpeed != clkSpeed 
      || pauseFlag != lastPausedFlag|| haltFlag != lastHaltFlag) {
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
    if (haltFlag) {
      u8g2.drawStr(0, 45, "HALTED");
    } else if (pauseFlag) {
      u8g2.drawStr(0, 45, "PAUSED");
    } 
    u8g2.sendBuffer();
    lastMode = mode;
    lastClkSpeed = clkSpeed;
    lastPausedFlag = pauseFlag;
    lastHaltFlag = haltFlag;
  }
}

void reset() {
  Serial.println("Reset...");
  delay(200);
  clkInc = 0;
  rst = true;
  irq = false;
  irqWait = false;
  irqSup = true;
  updateShift();
  delay(500); 
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
  rst = false;
  irq = false;
  irqWait = false;
  irqSup = true;
  clkClr = false;
  clkInc = 0;
  clkSpeed = 0;
  mode = MODE_STEP;
  lastClkSpeed = -1;
  lastMode = MODE_NULL; 
  updateShift();
  updateScreen();
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

  // Set up the control pins
  pinMode(MODE_PIN, INPUT);
  pinMode(UP_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(STEP_PIN, INPUT);
  pinMode(CLK_CLR_PIN, INPUT);
  pinMode(IRQ_PIN, INPUT);
  pinMode(IRQWAIT_CLR_PIN, INPUT);
  pinMode(IRQSUP_SET_PIN, INPUT);
  pinMode(IRQSUP_CLR_PIN, INPUT);
  pinMode(RST_PIN, INPUT);
  pinMode(PAUSE_PIN, INPUT);
  pinMode(HALT_PIN, INPUT);

  // Reset the system
  delay(500);
  reset();
}


void checkModeButton() {
  if (digitalRead(MODE_PIN)) {
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
        clkSpeed = clkSpeed + 1;
        if (clkSpeed > MAX_CLOCK_DELAYS) {
          clkSpeed = MAX_CLOCK_DELAYS;
        }
      }
    } else {
      upButtonPressedCounter++;
      if (upButtonPressedCounter > 50) {
        clkSpeed = clkSpeed + 1;
        if (clkSpeed > MAX_CLOCK_DELAYS) {
          clkSpeed = MAX_CLOCK_DELAYS;
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
        clkSpeed = clkSpeed - 1;
        if (clkSpeed < 0) {
          clkSpeed = 0;
        }
      }
    } else {
      downButtonPressedCounter++;
      if (downButtonPressedCounter > 50) {
        clkSpeed = clkSpeed - 1;
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
    if (digitalRead(STEP_PIN)) {
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
    if (digitalRead(IRQ_PIN)) {
      if (!irqButtonPressed) {
        irqButtonPressed = true;
        irqButtonCounter = 0;
      } else {
        irqButtonCounter++;
        if (irqButtonCounter > NUMBER_OF_TIMES_TO_CHECK_IRQ_BUTTON_PIN) {
          irqButtonCounter = 0;
          irqWait = true;
          updateShift();
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
      rstButtonPressed = true;
      rstButtonCounter = 0;
    } else {
      rstButtonCounter++;
      if (rstButtonCounter > NUMBER_OF_TIMES_TO_CHECK_RST_BUTTON_PIN) {
        rstButtonCounter = 0;
        reset();
      }
    }
  } else {
    if (rstButtonPressed) {
      rstButtonCounter++;
      if (rstButtonCounter > NUMBER_OF_TIMES_TO_CHECK_RST_BUTTON_PIN) {
        rstButtonPressed = false;
        rstButtonCounter = 0;
        updateShift();
      }
    }
  }
}

void checkIrqSupSet() {
  if (digitalRead(IRQSUP_SET_PIN)) {
    if (!irqSupSetPressed) {
      irqSup = true;
      irqSupSetPressed = true;
      updateShift();
    }
  } else {
    if (irqSupSetPressed) {
      irqSupSetPressed = false;
    }
  }
}

void checkIrqSupClr() {
  if (digitalRead(IRQSUP_CLR_PIN)) {
    if (!irqSupClrPressed) {
      irqSup = false;
      irqSupClrPressed = true;
      updateShift();
    }
  } else {
    if (irqSupClrPressed) {
      irqSupClrPressed = false;
    }
  }
}

void checkIrqWaitClr() {
  if (digitalRead(IRQWAIT_CLR_PIN)) {
    if (!irqWaitClrPressed) {
      irqWait = false;
      irqWaitClrPressed = true;
      updateShift();
    }
  } else {
    if (irqWaitClrPressed) {
      irqWaitClrPressed = false;
    }
  }
}

void checkClkClr() {
  if (digitalRead(CLK_CLR_PIN)) {
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

void checkPause() {
  if (digitalRead(PAUSE_PIN)) {
    pauseFlag = true;
    updateScreen();
    while(digitalRead(PAUSE_PIN)){}
    pauseFlag = false;
    updateScreen();
  }
}

void checkHalt() {
  if (digitalRead(HALT_PIN)) {
    haltFlag = true;
    updateScreen();
    while(true){} // Loop forever
  }
}

void loop() {
  checkHalt();
  checkPause();
  checkIrqSupClr();
  checkIrqSupSet();
  checkIrqWaitClr();
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
    if (millis() - lastClkInc >= ClockDelay[clkSpeed]) {
      updateClk();
    }
  }
  updateScreen();
}


