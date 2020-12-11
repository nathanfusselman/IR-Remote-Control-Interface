
// Nathan Fusselman
// 1001498232
// CSE 3442 - spModule

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "spModule.h"

//#define DEBUG

#define TIME 100000

#define DUTY 15000

// Bitband Aliases for input and output
#define SP_TX    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4)))

// PortE masks
#define SP_TX_MASK 16

// Initialize Hardware
void initSPM()
{
    // Enable clocks
        SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
        _delay_cycles(3);

        // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
        SYSCTL_GPIOHBCTL_R = 0;

        // Configure SP pins
        GPIO_PORTE_DIR_R |= SP_TX_MASK;
        GPIO_PORTE_DR2R_R |= SP_TX_MASK;
        GPIO_PORTE_DEN_R |= SP_TX_MASK;
        GPIO_PORTE_AFSEL_R |= SP_TX_MASK;
        GPIO_PORTE_PCTL_R &= GPIO_PCTL_PE4_M;
        GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_M1PWM2;

        //Configure PWM Gen
        SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;
        SYSCTL_SRPWM_R = 0;
        PWM1_1_CTL_R = 0;
        PWM1_1_GENA_R = PWM_1_GENA_ACTCMPAD_ZERO | PWM_1_GENA_ACTLOAD_ONE;
        PWM1_1_LOAD_R = DUTY;
        PWM1_INVERT_R = PWM_INVERT_PWM2INV;
        PWM1_1_CMPA_R = (DUTY/2) - 1;
        PWM1_1_CTL_R = PWM_1_CTL_ENABLE;
        PWM1_ENABLE_R = PWM_ENABLE_PWM2EN;

        GPIO_PORTE_DEN_R &= ~SP_TX_MASK;
}

void playNote(uint16_t freq, uint16_t dur) {
    uint16_t value = (40000000/2)/freq;
    PWM1_1_LOAD_R = value;
    PWM1_1_CMPA_R = (value/2) - 1;
    GPIO_PORTE_DEN_R |= SP_TX_MASK;
    waitMicrosecond(dur * 1000);
    GPIO_PORTE_DEN_R &= ~SP_TX_MASK;
}

void startupTone() {
    uint16_t startupfreq[] = {2489,1245,1865,1661,1245,2489,1865};
    uint16_t startupdur[] = {320,96,256,384,96,256,480};
    uint8_t i;
    for (i = 0; i < 7; i++) {
        playNote(startupfreq[i], startupdur[i]);
    }
}

void goodTone() {
    uint16_t goodfreq[] = {1760,3136};
    uint16_t gooddur[] = {100,100};
    uint8_t i;
    for (i = 0; i < 2; i++) {
        playNote(goodfreq[i], gooddur[i]);
    }
}

void badTone() {
    uint16_t badfreq[] = {3136,1760};
    uint16_t baddur[] = {100,100};
    uint8_t i;
    for (i = 0; i < 2; i++) {
        playNote(badfreq[i], baddur[i]);
    }
}

void errorTone() {
    uint16_t errorfreq[] = {880,0,880,0,880};
    uint16_t errordur[] = {100,25,100,25,100};
    uint8_t i;
    for (i = 0; i < 5; i++) {
        playNote(errorfreq[i], errordur[i]);
    }
}
