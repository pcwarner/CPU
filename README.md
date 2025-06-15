# CPU

This project is mainly for my own interest and has no real purpose apart from learning and to actually create a computer. It is a 8 data bit computer with 16 bit address bus. My have designed it from mostly 74HCxx logical chips, although at present I have some microcontrollers which emulate hardware. This is because it is easier to change the software in the controller than change the hardware.  At present this project is a "work in progress".

The way I have designed this computer is to first get the computer unit working in a logic emulator called "Digital". It is avilable at: https://github.com/hneemann/Digital/releases/tag/v0.31

I then design a PCB using the free version of DipTrace, which is available at: https://diptrace.com/download/download-diptrace/. The free version limits you to 300 pins and two layers, which is fine for the PCBs in this project. There are also other limitation for free version please seen their license for full details. I have recently upgraded to the "Non-profit" free version which permits upto 500 pins. This was done because the DECODE PCB now requires more than the 300 pin limit. 

After having the PCB made it may need additional iterations.   