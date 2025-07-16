import pandas as pd
import numpy as np
import math as math
import os

# Files to use
OUTPUT_INSTRUCTIONS_FILE = "D:\\CPU\\src\\arduino\\DECODE\\InstructionData.h"
INSTRUCTIONS_EXCEL_FILE = "D:\\CPU\src\\resources\\instructions\\InstructionsWithSteps.xlsx"

DATA_FILE0 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE0.hex"
DATA_FILE1 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE1.hex"
DATA_FILE2 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE2.hex"
DATA_FILE3 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE3.hex"
DATA_FILE4 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE4.hex"
DATA_FILE5 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE5.hex"
DATA_FILE6 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE6.hex"
DATA_FILE7 = "D:\\CPU\\src\\resources\\digital\\Data\\DECODE7.hex"

DATA_FILES = [DATA_FILE0, DATA_FILE1, DATA_FILE2, DATA_FILE3, DATA_FILE4, DATA_FILE5, DATA_FILE6, DATA_FILE7] 

ACTIONS = []
for i in range(0, 64):
  ACTIONS.append("")

# Define the actionsConstants
# Clock
ACTIONS[ 0] = "CLK_CLR"
ACTIONS[ 1] = "IRQSUP_SET"
ACTIONS[ 2] = "IRQSUP_CLR"
ACTIONS[ 3] = "IRQWAIT_CLR"
# Program Counter
ACTIONS[ 4] = "PCL_IN"
ACTIONS[ 5] = "PCL_SET"
ACTIONS[ 6] = "PCL_OUT"
ACTIONS[ 7] = "PCH_IN"
ACTIONS[ 8] = "PCH_SET"
ACTIONS[ 9] = "PCH_OUT"
ACTIONS[10] = "PC_IN"
ACTIONS[11] = "PC_SET"
ACTIONS[12] = "PC_OUT"
# Memory
ACTIONS[13] = "MEM_IN"
ACTIONS[14] = "MEM_OUT"
# Instruction Register
ACTIONS[15] = "IR_IN"
ACTIONS[16] = "IR_SET"
# Register A
ACTIONS[17] = "REGA_IN"
ACTIONS[18] = "REGA_SET"
ACTIONS[19] = "REGA_OUT"
# Register B
ACTIONS[20] = "REGB_IN"
ACTIONS[21] = "REGB_SET"
ACTIONS[22] = "REGB_OUT"
# Register S
ACTIONS[23] = "REGS_IN"
ACTIONS[24] = "REGS_SET"
ACTIONS[25] = "REGS_OUT"
# Flags also has carry 
ACTIONS[26] = "FLAGS_IN"
ACTIONS[27] = "FLAGS_SET"
ACTIONS[28] = "FLAGS_OUT"
ACTIONS[29] = "CIN_IN"
ACTIONS[30] = "CIN_VAL"
# Register C
ACTIONS[31] = "REGC_IN" 
ACTIONS[32] = "REGC_ALU"
ACTIONS[33] = "REGC_SET"
ACTIONS[34] = "REGC_OUT"
# Register F
ACTIONS[35] = "REGF_IN" 
ACTIONS[36] = "REGF_ALU"
ACTIONS[37] = "REGF_SET"
ACTIONS[38] = "REGF_OUT"
# Memory Pointer
ACTIONS[39] = "MPL_IN"
ACTIONS[40] = "MPL_SET"
ACTIONS[41] = "MPH_IN"
ACTIONS[42] = "MPH_SET"
ACTIONS[43] = "MP_OUT"
# Stack Pointer
ACTIONS[44] = "SPL_IN"
ACTIONS[45] = "SPL_SET"
ACTIONS[46] = "SPL_OUT"
ACTIONS[47] = "SPH_IN"
ACTIONS[48] = "SPH_SET"
ACTIONS[49] = "SPH_OUT"
ACTIONS[50] = "SP_OUT"
ACTIONS[51] = "SP_INC"
ACTIONS[52] = "SP_DEC"
# Interupt Vecter
ACTIONS[53] = "IVL_IN"
ACTIONS[54] = "IVL_SET"
ACTIONS[55] = "IVL_OUT"
ACTIONS[56] = "IVH_IN"
ACTIONS[57] = "IVH_SET"
ACTIONS[58] = "IVH_OUT"
# Not used
ACTIONS[59] = "NOT_USED1"
ACTIONS[60] = "NOT_USED2"
ACTIONS[61] = "NOT_USED3"
# Special actions
ACTIONS[62] = "HLT"
ACTIONS[63] = "RST"

for i in range(0, 64):
  if len(ACTIONS[i]) < 1:
    print("Action missing from array position=" + str(i))
    exit(-1)

# Global variables
data = []
instructionSteps = []

# Add data to array
def setData(ins, c, eq, step, instructionStep):
  class newInstructionStep: pass
  newInstructionStep.instructionNumber = ins
  newInstructionStep.stepNumber = step
  newInstructionStep.c = c
  newInstructionStep.eq = eq
  newInstructionStep.actionsString = instructionStep.actionsString
  newInstructionStep.actions = []
  for i in range(8):
     newInstructionStep.actions.append(instructionStep.actions[i])
  
  pos = ins        *  2 *  2 * 32
  pos = pos + c    *  2 * 32
  pos = pos + eq   * 32
  pos = pos + step
  data[pos] = newInstructionStep

# Fill the data array with blank elements
def createBlankData():
  for i in range(0, 256*2*2*32):
    data.append(0)

  for ins in range(0, 256):
    for c in range(0, 2):
      for eq in range(0, 2):
        for step in range(0, 32):
          class instructionStep: pass
          instructionStep.instructionNumber = ins
          instructionStep.stepNumber = step
          instructionStep.c = c
          instructionStep.eq = eq
          instructionStep.actions = []
          for i in range(0, 8):
            instructionStep.actions.append(0)
          if step % 2 == 0:
            instructionStep.actions[0] = 1
            instructionStep.actionsString = ACTIONS[0] # CLK_CLR
          else:
            instructionStep.actions[0] = 0
            instructionStep.actionsString = "<Null>"

          setData(ins, c, eq, step, instructionStep)

# Load the instructions from the Execel Spreadsheet
def loadInstructionsFromExcel():
  df = pd.read_excel(INSTRUCTIONS_EXCEL_FILE) 

  for y in range(len(df)):
    instructionNumber = df.loc[y, 'Instr No.'].astype(float)
    stepNumber = df.loc[y, 'Step No.'].astype(float)
    actions = df.loc[y , 'Actions']
    c = df.loc[y , 'C'].astype(float)
    eq = df.loc[y , 'EQ'].astype(float)

    if not np.isnan(instructionNumber):
      instructionNumber = int(instructionNumber)
      if not np.isnan(stepNumber):
        stepNumber = int(stepNumber)
        if isinstance(actions, str):
          if isinstance(c, float):
            if c == 1:
              c = 1
            else:
              c = 0
          else:
            c = 0
          if isinstance(eq, float):
            if eq == 1:
              eq = 1
            else:
              eq = 0
          else:
            c = 0

          if not ((c == 0 or c == 1) and (eq == 0 or eq == 1)):
            print("c or eq is vaildate: " + c + " " + eq)
            exit(-1)
          
          class instructionStep: pass
          instructionStep.instructionNumber = instructionNumber
          instructionStep.stepNumber = stepNumber
          instructionStep.c = c
          instructionStep.eq = eq
          instructionStep.actionsString = actions
          instructionStep.actions = []
          for i in range(8):
            instructionStep.actions.append(0)

          parts = actions.split(",")
          for a in parts:
            found = False
            for i in range(len(ACTIONS)):
              if a == ACTIONS[i]:
                j = i
                g = math.floor(j/8)
                r = j - (g*8)
                instructionStep.actions[g] += int(2**r)
                found = True
                break
            
            if not found:  
              print("Unknown action " + a + " at " + str(y))
              exit(-1)
          instructionSteps.append(instructionStep)


# This method fills the data array with the "NULL"
# instuction. Which is always the zero instruction.
# This way unassigned instructions will act as NULL.
def fillDataWithNullInstructions():
  for ins in range(256):
    for c in range(2):
      for eq in range(2):
        # Add the instruction step until not zero
        for s in range(32):
          if instructionSteps[s].instructionNumber != 0:
            break
          class instructionStep: pass
          instructionStep.instructionNumber = ins
          instructionStep.stepNumber = s
          instructionStep.c = c
          instructionStep.eq = eq
          instructionStep.actionsString = instructionSteps[s].actionsString
          instructionStep.actions = []
          for i in range(0, 8):
            instructionStep.actions.append(instructionSteps[s].actions[i])          
          setData(ins, c, eq, s, instructionStep)

# Add the instructions to the data
def processInstructionsToFillData():
  for ins in range(0, 256):
    for c in range(0, 2):
      for eq in range(0, 2):

        # Found instruction step the fully matches - If not found use the C=0 and EQ=0 version
        cToUse = 0
        eqToUse = 0
        found = False
        for instructionStep in instructionSteps:
          if instructionStep.instructionNumber == ins and instructionStep.c == c and instructionStep.eq == eq and instructionStep.stepNumber == 0:
            cToUse = c
            eqToUse = eq
            found = True
            break

        if not found and c == 1 and eq == 0:
          for instructionStep in instructionSteps:
            if instructionStep.instructionNumber == ins and instructionStep.c == 1 and instructionStep.eq == 0 and instructionStep.stepNumber == 0:
              cToUse = 1
              eqToUse = 0
              found = True
              break
         
        if not found and c == 0 and eq == 1:
          for instructionStep in instructionSteps:
            if instructionStep.instructionNumber == ins and instructionStep.c == 0 and instructionStep.eq == 1 and instructionStep.stepNumber == 0:
              cToUse = 0
              eqToUse = 1
              found = True
              break

        for s in range(0, 32):
          for instructionStep in instructionSteps:
            if instructionStep.instructionNumber == ins and instructionStep.c == cToUse and instructionStep.eq == eqToUse and instructionStep.stepNumber == s:
              setData(ins, c, eq, s, instructionStep)
              break


def createOutputInstructionsfile():
  if os.path.isfile(OUTPUT_INSTRUCTIONS_FILE):
    os.remove(OUTPUT_INSTRUCTIONS_FILE)
  with open(OUTPUT_INSTRUCTIONS_FILE, 'w') as f:
    f.write("// Generated code - Please do not edit\n")
    f.write('const uint8_t INSTRUCTIONS[] = {\n')
    for i in range(0, 256*2*2*32):
      f.write("  "\
            + "{0:#0{1}x}".format(data[i].actions[0], 4) + ", " + "{0:#0{1}x}".format(data[i].actions[1], 4) + ", "\
            + "{0:#0{1}x}".format(data[i].actions[2], 4) + ", " + "{0:#0{1}x}".format(data[i].actions[3], 4) + ", "\
            + "{0:#0{1}x}".format(data[i].actions[4], 4) + ", " + "{0:#0{1}x}".format(data[i].actions[5], 4) + ", "\
            + "{0:#0{1}x}".format(data[i].actions[6], 4) + ", " + "{0:#0{1}x}".format(data[i].actions[7], 4) + \
            ", // " + str(data[i].instructionNumber) + ","\
            + str(data[i].stepNumber) + ": C=" + str(data[i].c) + " EQ=" + str(data[i].eq) + " A=" + data[i].actionsString +"\n")
    f.write("};\n")

def createOutputDataFiles():
  for i in range(0, 8):
    if os.path.isfile(DATA_FILES[i]):
      os.remove(DATA_FILES[i])
    with open(DATA_FILES[i], 'w') as f:
      f.write("v2.0 raw\n")
      for j in range(0, 256*2*2*32):
        f.write("{0:#0{1}x}".format(data[j].actions[i], 4) + "\n")

# Main function
def main():

  loadInstructionsFromExcel()

  createBlankData()
  fillDataWithNullInstructions()
  
  processInstructionsToFillData()

  createOutputInstructionsfile()
  createOutputDataFiles()

if __name__=="__main__":
    main()