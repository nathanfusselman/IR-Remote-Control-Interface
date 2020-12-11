// spModule Library - Lab8
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

#ifndef SPMODULE_H_
#define SPMODULE_H_

void initSPM(void);
void waitMicrosecond(uint32_t);
void startupTone(void);
void goodTone(void);
void badTone(void);
void errorTone(void);
void playNote(uint16_t, uint16_t);


#endif /* SPMODULE_H_ */
