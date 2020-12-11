// irDecoder Library - Lab6
// Nathan Fusselman

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef IRDECODER_H_
#define IRDECODER_H_

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void setTimerInterrupt1(uint32_t);
void timerInterrupt1(void);
uint8_t getBit(void);
uint8_t invertBit(uint8_t);
bool checkError(void);
uint8_t bToI(uint8_t []);
void parseBuffer(void);
void parseRAW(void);
void fallingEdge(void);
void initIRD(void);
void sendButtonNum(uint8_t);

#endif
