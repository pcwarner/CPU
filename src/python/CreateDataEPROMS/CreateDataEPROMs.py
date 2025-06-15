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

# Constants
CLK_CLR     =  0
PCL_IN      =  1
PCL_SET     =  2
PCL_OUT     =  3
PCH_IN      =  4
PCH_SET     =  5
PCH_OUT     =  6
PC_IN       =  7
PC_SET      =  8
PC_OUT      =  9
MEM_IN      = 10
MEM_OUT     = 11
IR_IN       = 12
IR_SET      = 13
IRQSUP_SET  = 14
IRQSUP_CLR  = 15
IRQWAIT_CLR = 16
REGA_IN     = 17
REGA_SET    = 18
REGA_OUT    = 19
REGB_IN     = 20
REGB_SET    = 21
REGB_OUT    = 22
REGS1_IN    = 23
REGS1_SET   = 24
REGS1_OUT   = 25
REGS2_IN    = 26
REGS2_SET   = 27
REGS2_OUT   = 28
REGC_IN     = 29 # Also does REGF_IN
REGC_SET    = 30 # Also does REGF_SET
REGC_OUT    = 31
REGF_OUT    = 32
CIN_SET     = 33
CIN_CLR     = 34
MPL_IN      = 35
MPL_SET     = 36
MPH_IN      = 37
MPH_SET     = 38
MP_OUT      = 39
SPL_IN      = 40
SPL_SET     = 41
SPL_OUT     = 42
SPH_IN      = 43
SPH_SET     = 44
SPH_OUT     = 45
SP_OUT      = 46
SP_INC      = 47
SP_DEC      = 48
IVL_IN      = 49
IVL_SET     = 50
IVL_OUT     = 51
IVH_IN      = 52
IVH_SET     = 53
IVH_OUT     = 54
RST         = 63

CLK_CLR_STR     = "CLK_CLR"
PCL_IN_STR      = "PCL_IN"
PCL_SET_STR     = "PCL_SET"
PCL_OUT_STR     = "PCL_OUT"
PCH_IN_STR      = "PCH_IN"
PCH_SET_STR     = "PCH_SET"
PCH_OUT_STR     = "PCH_OUT"
PC_IN_STR       = "PC_IN"
PC_SET_STR      = "PC_SET"
PC_OUT_STR      = "PC_OUT"
MEM_IN_STR      = "MEM_IN"
MEM_OUT_STR     = "MEM_OUT"
IR_IN_STR       = "IR_IN"
IR_SET_STR      = "IR_SET"
IRQSUP_SET_STR  = "IRQSUP_SET"
IRQSUP_CLR_STR  = "IRQSUP_CLR"
IRQWAIT_CLR_STR = "IRQWAIT_CLR"
REGA_IN_STR     = "REGA_IN"
REGA_SET_STR    = "REGA_SET"
REGA_OUT_STR    = "REGA_OUT"
REGB_IN_STR     = "REGB_IN"
REGB_SET_STR    = "REGB_SET"
REGB_OUT_STR    = "REGB_OUT"
REGS1_IN_STR    = "REGS1_IN"
REGS1_SET_STR   = "REGS1_SET"
REGS1_OUT_STR   = "REGS1_OUT"
REGS2_IN_STR    = "REGS2_IN"
REGS2_SET_STR   = "REGS2_SET"
REGS2_OUT_STR   = "REGS2_OUT"
REGC_IN_STR     = "REGC_IN"
REGC_SET_STR    = "REGC_SET"
REGC_OUT_STR    = "REGC_OUT"
REGF_OUT_STR    = "REGF_OUT"
CIN_SET_STR     = "CIN_SET"
CIN_CLR_STR     = "CIN_CLR"
MPL_IN_STR      = "MPL_IN"
MPL_SET_STR     = "MPL_SET"
MPH_IN_STR      = "MPH_IN"
MPH_SET_STR     = "MPH_SET"
MP_OUT_STR      = "MP_OUT"
SPL_IN_STR      = "SPL_IN"
SPL_SET_STR     = "SPL_SET"
SPL_OUT_STR     = "SPL_OUT"
SPH_IN_STR      = "SPH_IN"
SPH_SET_STR     = "SPH_SET"
SPH_OUT_STR     = "SPH_OUT"
SP_OUT_STR      = "SP_OUT"
SP_INC_STR      = "SP_INC"
SP_DEC_STR      = "SP_DEC"
IVL_IN_STR      = "IVL_IN"
IVL_SET_STR     = "IVL_SET"
IVL_OUT_STR     = "IVL_OUT"
IVH_IN_STR      = "IVH_IN"
IVH_SET_STR     = "IVH_SET"
IVH_OUT_STR     = "IVH_OUT"
RST_STR         = "RST"

INSTR_VALUES = [CLK_CLR, PCL_IN,PCL_SET, PCL_OUT, PCH_IN, PCH_SET, PCH_OUT, PC_IN, PC_SET, PC_OUT, \
  MEM_IN, MEM_OUT, IR_IN, IR_SET, IRQSUP_SET, IRQSUP_CLR, IRQWAIT_CLR, REGA_IN, REGA_SET, REGA_OUT, \
  REGB_IN, REGB_SET, REGB_OUT, REGS1_IN, REGS1_SET, REGS1_OUT, REGS2_IN, REGS2_SET, REGS2_OUT, \
  REGC_IN, REGC_SET, REGC_OUT, REGF_OUT, CIN_SET, CIN_CLR, MPL_IN, MPL_SET, MPH_IN, MPH_SET, MP_OUT, \
  SPL_IN, SPL_SET, SPL_OUT, SPH_IN, SPH_SET, SPH_OUT, SP_OUT, SP_INC, SP_DEC, IVL_IN, IVL_SET, \
  IVL_OUT, IVH_IN, IVH_SET, IVH_OUT, RST]
INSTR_STRS  = [CLK_CLR_STR, PCL_IN_STR, PCL_SET_STR, PCL_OUT_STR, PCH_IN_STR, PCH_SET_STR, PCH_OUT_STR, \
  PC_IN_STR, PC_SET_STR, PC_OUT_STR, MEM_IN_STR,   MEM_OUT_STR, IR_IN_STR, IR_SET_STR, IRQSUP_SET_STR, \
  IRQSUP_CLR_STR, IRQWAIT_CLR_STR, REGA_IN_STR, REGA_SET_STR, REGA_OUT_STR, REGB_IN_STR, REGB_SET_STR, \
  REGB_OUT_STR, REGS1_IN_STR, REGS1_SET_STR, REGS1_OUT_STR, REGS2_IN_STR, REGS2_SET_STR, REGS2_OUT_STR, \
  REGC_IN_STR, REGC_SET_STR, REGC_OUT_STR, REGF_OUT_STR, CIN_SET_STR, CIN_CLR_STR, MPL_IN_STR, MPL_SET_STR, \
  MPH_IN_STR, MPH_SET_STR, MP_OUT_STR, SPL_IN_STR, SPL_SET_STR, SPL_OUT_STR, SPH_IN_STR, SPH_SET_STR, \
  SPH_OUT_STR, SP_OUT_STR, SP_INC_STR, SP_DEC_STR, IVL_IN_STR, IVL_SET_STR, IVL_OUT_STR, IVH_IN_STR, \
  IVH_SET_STR, IVH_OUT_STR, RST_STR]
if len(INSTR_VALUES) != len(INSTR_STRS):
  print("Instruction arrays not same length")
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
            instructionStep.actionsString = CLK_CLR_STR
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
            for i in range(len(INSTR_STRS)):
              if a == INSTR_STRS[i]:
                j = INSTR_VALUES[i]
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