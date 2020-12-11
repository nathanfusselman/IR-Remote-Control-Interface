// irEncoder Library - Lab7
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

#ifndef IRENCODER_H_
#define IRENCODER_H_

void initIRE(void);
void timerInterrupt0(void);
void setTimerInterrupt0(uint32_t);
void sendData(bool[], bool[]);
void sendDataError(bool[], bool[]);
void sendError();
void sendButton(uint8_t, uint8_t);
void sendButtonNum(uint8_t);


#endif /* IRENCODER_H_ */
