// Nathan Fusselman
// 1001498232
// CSE 3442 - irDecoder

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
#include "irDecoder.h"
#include "uart0.h"

//#define DEBUG

#define BAUD 562

#define BAUD_OFFSET 0
#define PREAMBLE_OFFSET_1 0
#define PREAMBLE_OFFSET_2 0

// Bitband Aliases for input and output
#define IR_RX    (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4)))
#define IR_DB    (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 0*4)))

// PortB masks
#define IR_RX_MASK 32
#define IR_DB_MASK 1

// Samples Numbers
#define SAMPLES_PER 3               // number of samples per state.
#define MIN_SAME 2
#define MAX_SAME 4
#define MAX_SAMPLES 102             // total num of samples
#define INIT_SAMPLES 6              // size of first samples

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint16_t testIndex;
bool rawData[((MAX_SAMPLES - INIT_SAMPLES) * SAMPLES_PER) +1];
bool bufferIR[(MAX_SAMPLES - INIT_SAMPLES) + 1];
uint16_t sampleNum;
uint8_t addr [8];
uint8_t data [8];
uint8_t addri [8];
uint8_t datai [8];
bool one = true;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Configure Timer 1
void setTimerInterrupt1(uint32_t microSec)
{
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring, =1
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;          // configure as 32-bit counter (A + B) -> CFG register, page 728, TIMER1_CFG_R = 0; is the same thing
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;         // One shot mode (TAMR reg page 732), or = 1 -> same thing
    TIMER1_TAILR_R = 40 * microSec;                 // 40 cycles per micro sec
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                // turn-on interrupts, = 1
    NVIC_EN0_R |= 1 << (INT_TIMER1A-16);            // turn-on interrupt 37, page 104
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on timer, = 1
}

// When the interrupt is called (or hit)
// Modified _ccs.c file too
void timerInterrupt1()
{
    TIMER1_ICR_R |= 1;                              // clear interrupt every time fn is called so it doesn't repeat itself

    // Test pts
    uint32_t widthSample = (BAUD + BAUD_OFFSET) / SAMPLES_PER;
    uint32_t deltaSamples [] = {2250, 2250, 2250, 3750 + PREAMBLE_OFFSET_1, 1500, 1500 + PREAMBLE_OFFSET_2 + widthSample/2};
    IR_DB ^= 1;                                     // for test on the oscilloscope
    bool error = false;

    if(testIndex <= (MAX_SAMPLES-INIT_SAMPLES) * SAMPLES_PER + INIT_SAMPLES)
    {
        if(testIndex < INIT_SAMPLES)
        {
            switch(testIndex)
            {
                case 1:
                case 2:
                case 3:
                    if(IR_RX != 0)                  // Not Low
                    {
                        error = true;
                        testIndex = MAX_SAMPLES;
                    }
                    break;
                case 4:
                case 5:
                    if(IR_RX != 1)                  // Not High
                    {
                        error = true;
                        testIndex = MAX_SAMPLES;
                    }
                    break;
            }
            if(!error)
                setTimerInterrupt1(deltaSamples[testIndex]);
        }
        else
        {
                setTimerInterrupt1(widthSample);
                rawData[testIndex - INIT_SAMPLES] = IR_RX;
                #ifdef DEBUG
                if (IR_RX)
                    putcUart0('1');
                else
                    putcUart0('0');
                #endif
        }
        testIndex++;
    }
    else
    {
        #ifdef DEBUG
        putcUart0('\n');
        putcUart0('\n');
        #endif
        parseRAW();
        #ifdef DEBUG
        putcUart0('\n');
        putcUart0('\n');
        #endif
        parseBuffer();
        GPIO_PORTB_ICR_R |= IR_RX_MASK;         // clear interrupt
        GPIO_PORTB_IM_R |= IR_RX_MASK;          // turn falling edge interrupt back on
    }

    if(error)                                   // Frame preamble is wrong
    {
        TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
        GPIO_PORTB_ICR_R |= IR_RX_MASK;         // clear interrupt
        GPIO_PORTB_IM_R |= IR_RX_MASK;          // turn falling edge interrupt back on
        return;
    }
 }

void parseRAW()
{
    uint16_t bit = 0;
    uint16_t i = 0;
    bool current = rawData[0];
    uint8_t num = 0;
    for (i = 0; i < (MAX_SAMPLES - INIT_SAMPLES) * SAMPLES_PER; i++) {
        if (rawData[i] == current) {
            num++;
        } else {
            if (num >= MIN_SAME && num <= MAX_SAME) {
                #ifdef DEBUG
                if (current)
                    putcUart0('1');
                else
                    putcUart0('0');
                #endif
                bufferIR[bit] = current;
                bit++;
            }
            if (num > MAX_SAME) {
                uint8_t total = num/SAMPLES_PER;
                uint8_t j = 0;
                if (!(num%2)) {
                    total++;
                }
                for (j = 0; j < total; j++) {
                    #ifdef DEBUG
                    if (current)
                        putcUart0('1');
                    else
                        putcUart0('0');
                    #endif
                    bufferIR[bit] = current;
                    bit++;
                }
            }
            num = 1;
            current = rawData[i];
        }
    }
}

// Modified _ccs.c file too
void fallingEdge()
{
    GPIO_PORTB_ICR_R |= IR_RX_MASK;                  // clear interrupt
    GPIO_PORTB_IM_R &= ~IR_RX_MASK;                  // turn-off key press interrupts

    testIndex = 0;                                   // start
    timerInterrupt1();                               // call interrupt fn
}


uint8_t invertBit(uint8_t bit)
{
    if (bit == 0)
        return 1;
    if (bit == 1)
        return 0;
    return 255;
}

// Binary to decimal/Integer conversion
uint8_t bToI(uint8_t byte[8])
{
    uint8_t i;
    uint8_t mult = 1;
    uint8_t result = 0;

    for(i = 0; i < 8; i++)
    {
        result += byte[7 - i] * mult;
        mult *= 2;
    }
    return result;
}

bool checkError()
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        // Check correct bit
        if (!(addr[i] == 0 || addr[i] == 1))
            return true;
        if (!(addri[i] == 0 || addri[i] == 1))
            return true;
        if (!(data[i] == 0 || data[i] == 1))
            return true;
        if (!(datai[i] == 0 || datai[i] == 1))
            return true;
        // Check the complement
        if (addr[i] != invertBit(addri[i]))
            return true;
        if (data[i] != invertBit(datai[i]))
            return true;
    }
    return false;
}

// 01 = 0, 0111 = 1
uint8_t getBit()
{
    if(bufferIR[sampleNum] == 0 && bufferIR[sampleNum + 1] == 1 && bufferIR[sampleNum + 2] == 0) // _| |_
    {
        sampleNum += 2;
        #ifdef DEBUG
        putcUart0('0');
        #endif
        return 0;
    }
    if(bufferIR[sampleNum] == 0 && bufferIR[sampleNum + 1] == 1 && bufferIR[sampleNum + 2] == 1
            && bufferIR[sampleNum + 3] == 1 && bufferIR[sampleNum + 4] == 0)                    // _|    |_
    {
        sampleNum += 4;
        #ifdef DEBUG
        putcUart0('1');
        #endif
        return 1;
    }
    #ifdef DEBUG
    putcUart0('E');
    #endif
    return 255;
}

// Test the samples and show the matching button
void parseBuffer()
{
    uint8_t bitNum = 0;
    sampleNum = 0;
    for(bitNum = 0; bitNum < 32; bitNum++)
    {
        if(bitNum < 8)
            addr[bitNum] = getBit();
        if(bitNum >= 8 && bitNum < 16)
            addri[bitNum - 8] = getBit();
        if(bitNum >= 16 && bitNum < 24)
            data[bitNum - 16] = getBit();
        if(bitNum >= 24 && bitNum < 32)
            datai[bitNum - 24] = getBit();
    }
    #ifdef DEBUG
    putcUart0('\n');
    putcUart0('\n');
    #endif

    if(!checkError())
    {
        dataReturn(bToI(addr), bToI(data), false);
    } else {
        dataReturn(bToI(addr), bToI(data), true);
        errorAlert();
    }
}

// Initialize Hardware
void initIRD()
{
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Configure IR pins
    GPIO_PORTB_DIR_R |= IR_DB_MASK;
    GPIO_PORTB_DIR_R &= ~IR_RX_MASK;
    GPIO_PORTB_DR2R_R |= IR_DB_MASK;                 // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTB_DEN_R |= IR_RX_MASK | IR_DB_MASK;

    // Configure falling edge interrupts on row inputs
    // (edge mode, single edge, falling edge, clear any interrupts, turn on)
    GPIO_PORTB_IS_R &= ~IR_RX_MASK;                  //turning on falling edge interrupts
    GPIO_PORTB_IBE_R &= ~IR_RX_MASK;
    GPIO_PORTB_IEV_R &= ~IR_RX_MASK;
    GPIO_PORTB_ICR_R |= IR_RX_MASK;
    NVIC_EN0_R |= 1 << (INT_GPIOB-16);               // turn-on interrupt 17 (GPIOB) -> TABLE 2.9 PG 104
    GPIO_PORTB_IM_R |= IR_RX_MASK;                   // turn-on interrupt mask

    IR_DB = 1;                                       // test with oscilloscope
}
