// Nathan Fusselman
// 1001498232
// CSE 3442 - Lab9

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "irDecoder.h"
#include "irEncoder.h"
#include "spModule.h"
#include "eeprom.h"

#define MAX_STRING_LENGTH 16

void initHw(void);
int main(void);
void goodAlert(void);
void badAlert(void);
void errorAlert(void);
uint8_t getButton(uint8_t, uint8_t);
void dataReturn(uint8_t, uint8_t, bool);
void waitMicrosecond(uint32_t);

bool goodAlertEnable = true;
bool badAlertEnable = true;
bool errorAlertEnable = true;
bool learning = false;
bool decode = false;
char lastName[MAX_STRING_LENGTH];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    USER_DATA data;
    initHw();
    initIRD();
    initIRE();
    initSPM();
    initUart0();
    initEeprom();
    setUart0BaudRate(115200, 40e6);
    startupTone();

    while(true) {
        putsUart0("> ");
        getsUart0(&data);
        parseFields(&data);
        if (isCommand(&data, "HELP", 0)) {
            putsUart0("List of available commands:\n");
            putsUart0("\tHELP\n");
            putsUart0("\tLIST\n");
            putsUart0("\tDECODE [ON/OFF]\n");
            putsUart0("\tCLEAR\n");
            putsUart0("\tLEARN <NAME> [ADDR] [DATA]\n");
            putsUart0("\tINFO <NAME/ID>\n");
            putsUart0("\tERASE <NAME/ID>\n");
            putsUart0("\tPLAY <NAME> ...\n");
            putsUart0("\tSEND <ADDR> <DATA>\n");
            putsUart0("\tSENDB <BUTTON>\n");
            putsUart0("\tSENDE\n");
            putsUart0("\tALERT <GOOD/BAD/ERROR> <ON/OFF>\n");
        }
        if (isCommand(&data, "LIST", 0))
            printCommands();
        if (isCommand(&data, "DECODE", 0)) {
            if (isCommand(&data, "DECODE", 1)) {
                if (stringCompare(getFieldString(&data, 1),"ON"))
                    decode = true;
                else
                    decode = false;
            } else
                decode ^= 1;
        }
        if (isCommand(&data, "CLEAR", 0))
            clearAllCommands();
        if (isCommand(&data, "LEARN", 1)) {
            char * name = getFieldString(&data, 1);
            if (isCommand(&data, "LEARN", 3)) {
                addCommand(name, getFieldInteger(&data, 2), getFieldInteger(&data, 3));
                goodAlert();
                putsUart0("DONE\n");
            } else {
                learning = true;
                uint8_t i;
                for (i = 0; i < MAX_STRING_LENGTH; i++)
                    lastName[i] = name[i];
                while (learning);
            }
        }
        if (isCommand(&data, "INFO", 1)) {
            if (data.fieldType[1] == 'N')
                infoCommand(getFieldInteger(&data, 1));
            if (data.fieldType[1] == 'A')
                infoCommandName(getFieldString(&data, 1));
        }
        if (isCommand(&data, "ERASE", 1)) {
            if (data.fieldType[1] == 'N') {
                eraseCommand(getFieldInteger(&data, 1));
                putsUart0("Item has been removed.\n");
            }
            if (data.fieldType[1] == 'A') {
                char * name = getFieldString(&data, 1);
                eraseCommandName(name);
                putsUart0(name);
                putsUart0(": Has been removed.\n");
            }
        }
        if (isCommand(&data, "PLAY", 1)) {
            uint8_t i;
            uint8_t count = data.fieldCount;
            for (i = 0; i < count - 1; i++) {
                char * name = getFieldString(&data, i+1);
                uint16_t adddata = getCommand(name);
                uint8_t addr = adddata >> 8;
                uint8_t data = ((adddata << 8) >> 8);
                putsUart0("Played: ");
                putsUart0(name);
                putcUart0('\n');
                sendButton(addr, data);
                waitMicrosecond(100000);
            }
        }
        if (isCommand(&data, "SEND", 2))
            sendButton(getFieldInteger(&data, 1), getFieldInteger(&data, 2));
        if (isCommand(&data, "SENDB", 1))
            sendButtonNum(getFieldInteger(&data, 1));
        if (isCommand(&data, "SENDE", 0))
            sendError();
        if (isCommand(&data, "ALERT", 2)) {
            if (stringCompare(getFieldString(&data, 1),"GOOD")) {
                if (stringCompare(getFieldString(&data, 2),"ON"))
                    goodAlertEnable = true;
                else
                    goodAlertEnable = false;
            }
            if (stringCompare(getFieldString(&data, 1),"BAD")) {
                if (stringCompare(getFieldString(&data, 2),"ON"))
                    badAlertEnable = true;
                else
                    badAlertEnable = false;
            }
            if (stringCompare(getFieldString(&data, 1),"ERROR")) {
                if (stringCompare(getFieldString(&data, 2),"ON"))
                    errorAlertEnable = true;
                else
                    errorAlertEnable = false;
            }
        }
    }
}

void goodAlert() {
    if (goodAlertEnable)
        goodTone();
}

void badAlert() {
    if (badAlertEnable)
        badTone();
}

void errorAlert() {
    if (errorAlertEnable)
        errorTone();
}

// Match data with respective button number
uint8_t getButton(uint8_t addr, uint8_t data)
{
    uint8_t buttons[] = {162, 98, 226, 34, 2, 194, 224, 168, 144, 104, 152, 176, 48, 24, 122, 16, 56, 90, 66, 74, 82};
    uint8_t i;
    if(addr == 0)  {
        for(i = 0; i < 21; i++) {
            if(data == buttons[i])
                return i+1;
        }
    }
    return 255;
}

void dataReturn(uint8_t addr, uint8_t data, bool error) {
    if (!error && !learning)
        putsUart0("\n");
    if (learning) {
        if (!error) {
            goodAlert();
            addCommand(lastName, addr, data);
            putsUart0("DONE\n");
            learning = false;
        } else {
            errorAlert();
            putsUart0("ERROR\n");
            learning = false;
        }
    } else {
        uint8_t button = getButton(addr, data);
        uint8_t result = findCommand(addr, data);
        if(button < 255) {
            putsUart0("Button ");
            putiUart0(button);
            goodAlert();
            putcUart0('\n');
            if (result != 255) {
                goodAlert();
                getName(result);
            }
        } else {
            if (result != 255) {
                goodAlert();
                getName(result);
            } else
                badAlert();
        }
        if (decode) {
            uint8_t i;
            putsUart0("ADDRESS: ");
            for (i = 0; i < 8; i++)
                putiUart0((addr >> i) & 1);
            putcUart0('\n');
            putsUart0("DATA: ");
            for (i = 0; i < 8; i++)
                putiUart0((data >> i) & 1);
            putcUart0('\n');
        }
    }
    if (!error)
            putsUart0("> ");
}

// Approximate busy waiting (in units of microseconds), given a 40 MHz system clock
void waitMicrosecond(uint32_t us)
{
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*2 (speculative, so P=1)
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             NOP");                  // 1
    __asm("             B    WMS_LOOP0");       // 1*2 (speculative, so P=1)
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}
