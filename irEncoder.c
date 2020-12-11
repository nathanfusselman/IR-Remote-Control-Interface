// Nathan Fusselman
// 1001498232
// CSE 3442 - irEncoder

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
#include "irEncoder.h"
#include "uart0.h"

//#define DEBUG

#define INITLOW 9000
#define INITHIGH 4500
#define BAUD 562

#define DUTY 1050

#define DEFAULT_ADDR 0

// Bitband Aliases for input and output
#define IR_TX    (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 6*4)))

// PortB masks
#define IR_TX_MASK 64

bool dataOut[100];
uint8_t i;

// Initialize Hardware
void initIRE()
{

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Configure IR pins
    GPIO_PORTB_DIR_R |= IR_TX_MASK;
    GPIO_PORTB_DR2R_R |= IR_TX_MASK;
    GPIO_PORTB_DEN_R |= IR_TX_MASK;
    GPIO_PORTB_AFSEL_R |= IR_TX_MASK;
    GPIO_PORTB_PCTL_R &= GPIO_PCTL_PB6_M;
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB6_M0PWM0;

    //Configure PWM Gen
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;
    SYSCTL_SRPWM_R = 0;
    PWM0_0_CTL_R = 0;
    PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTLOAD_ONE;
    PWM0_0_LOAD_R = DUTY;
    PWM0_INVERT_R = PWM_INVERT_PWM0INV;
    PWM0_0_CMPA_R = (DUTY/2) -1;
    PWM0_0_CTL_R = PWM_0_CTL_ENABLE;
    PWM0_ENABLE_R = PWM_ENABLE_PWM0EN;

    GPIO_PORTB_DEN_R &= ~IR_TX_MASK;
}

// Configure Timer 0
void setTimerInterrupt0(uint32_t microSec)
{
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring, = 1
    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;          // configure as 32-bit counter (A + B) -> CFG register, page 728, TIMER0_CFG_R = 0; is the same thing
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;         // One shot mode (TAMR reg page 732), or = 1 -> same thing
    TIMER0_TAILR_R = 40 * microSec;                 // 40 cycles per micro sec
    TIMER0_IMR_R = TIMER_IMR_TATOIM;                // turn-on interrupts, = 1
    NVIC_EN0_R |= 1 << (INT_TIMER0A-16);            // turn-on interrupt 36, page 104
    TIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn-on timer, = 1
}

// When the interrupt is called (or hit)
// Modified _ccs.c file too
void timerInterrupt0()
{
    TIMER0_ICR_R |= 1;                              // clear interrupt every time fn is called so it doesn't repeat itself
    if (i < 100) {
        switch (i) {
        case 0:
            setTimerInterrupt0(INITLOW);
            break;
        case 1:
            setTimerInterrupt0(INITHIGH);
            break;
        default:
            setTimerInterrupt0(BAUD);
            break;
        }
        if (!dataOut[i]) {
            #ifdef DEBUG
            putcUart0('1');
            #endif
            GPIO_PORTB_DEN_R |= IR_TX_MASK;
        } else {
            #ifdef DEBUG
            putcUart0('0');
            #endif
            GPIO_PORTB_DEN_R &= ~IR_TX_MASK;
        }
        i++;
    }
    #ifdef DEBUG
    if (i == 100) {
        putcUart0('\n');
    }
    #endif
}

void sendData(bool addr[8], bool data[8]) {
    uint8_t j = 2;
    dataOut[0] = 0;
    dataOut[1] = 1;
    for (i = 0; i < 8; i++) {
        if (addr[i]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    for (i = 0; i < 8; i++) {
        if (!addr[i]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    for (i = 0; i < 8; i++) {
        if (data[i]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    for (i = 0; i < 8; i++) {
        if (!data[i]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    dataOut[j] = 0;
    dataOut[j+1] = 1;
    i = 0;
    timerInterrupt0();
}

void sendDataError(bool addr[8], bool data[8]) {
    uint8_t j = 2;
    dataOut[0] = 0;
    dataOut[1] = 1;
    for (i = 0; i < 8; i++) {
        if (addr[i]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    for (i = 8; i > 0; i--) {
        if (!addr[i-1]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    for (i = 0; i < 8; i++) {
        if (data[i]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    for (i = 8; i > 0; i--) {
        if (!data[i-1]) {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            dataOut[j+2] = 1;
            dataOut[j+3] = 1;
            j += 4;
        } else {
            dataOut[j] = 0;
            dataOut[j+1] = 1;
            j += 2;
        }
    }
    dataOut[j] = 0;
    dataOut[j+1] = 1;
    i = 0;
    timerInterrupt0();
}

void sendButton(uint8_t addrd, uint8_t datad) {
    bool addr[] = {0,0,0,0,0,0,0,0};
    bool data[] = {0,0,0,0,0,0,0,0};
    uint8_t bit;
    uint8_t test = addrd;
    for (bit = 8; bit > 0; --bit) {
        addr[8-bit] = (1 << (bit-1)) & test;
    }
    test = datad;
    for (bit = 8; bit > 0; --bit) {
        data[8-bit] = (1 << (bit-1)) & test;
    }
    sendData(addr, data);
}

void sendError() {
    bool addr[] = {0,1,0,1,0,1,0,1};
    bool data[] = {1,0,1,0,1,0,1,0};
    sendDataError(addr, data);
}

void sendButtonNum(uint8_t button) {
    uint8_t buttons[] = {162, 98, 226, 34, 2, 194, 224, 168, 144, 104, 152, 176, 48, 24, 122, 16, 56, 90, 66, 74, 82};
    sendButton(0, buttons[button-1]);

}
