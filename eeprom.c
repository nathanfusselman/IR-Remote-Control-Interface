// EEPROM functions
// Nathan Fusselman

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    -

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "eeprom.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

//Provided function by my main man Losh that initializes the EEPROM Module.
void initEeprom()
{
    SYSCTL_RCGCEEPROM_R = 1;
    _delay_cycles(3);
    while (EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
}

//Provided function by my main man Losh that writes a 32bit word to the EEPROM at a 16bit address location.
void writeEeprom(uint16_t add, uint32_t data)
{
    EEPROM_EEBLOCK_R = add >> 4;
    EEPROM_EEOFFSET_R = add & 0xF;
    EEPROM_EERDWR_R = data;
    while (EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
}

//Provided function by my main man Losh that reads a 32bit word from the EEPROM at a 16bit address location.
uint32_t readEeprom(uint16_t add)
{
    EEPROM_EEBLOCK_R = add >> 4;
    EEPROM_EEOFFSET_R = add & 0xF;
    return EEPROM_EERDWR_R;
}

//Sets the Size of saved commands to 0 so that it will start rewriting at the first block in the EEPROM.
void clearAllCommands()
{
    writeEeprom(SIZE_LOC, 0);
}

//Creates and adds block to EEPROM containing the command data.
void addCommand(char * name, uint8_t add, uint8_t data)
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint16_t start = (size * ((MAX_STRING_LENGTH / 4) + 1)) + 1;        //Calculates the current starting location (size * blocksize) + 1
    uint8_t i;
    uint32_t temp[((MAX_STRING_LENGTH / 4) + 1)];                       //Declares temporary block in memory
    for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)                 //Initializes temporary block to all 0
        temp[i] = 0;
    for (i = 0; name[i] != '\0' && i < MAX_STRING_LENGTH; i++)          //Iterates through name until \0 or MAX_STRING_LENGTH
        temp[i / 4] |= name[i] << ((4 - 1) - (i % 4)) * 8;              //Shifts char to the left by (i % 4) * 8 and sets that in word = i / 4
    temp[((MAX_STRING_LENGTH / 4) + 1) - 1] = add << 8 | data;          //Sets last word in block to contain the addr and data | 00000000 | 00000000 | addr | data|
    for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)                 //Writes temporary data to EEPROM
        writeEeprom(start + i, temp[i]);
    writeEeprom(SIZE_LOC, size + 1);                                    //Increments the saved size in EEPROM
}

//Returns the entry index of a block saved in the EEPROM
uint8_t getIndex(char * name)
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint8_t index;
    for (index = 0; index < size; index++) {                            //Increments through each block saved in EEPROM
        uint16_t start = (index * ((MAX_STRING_LENGTH / 4) + 1)) + 1;   //Calculates the current starting location (size * blocksize) + 1
        uint8_t i;
        uint32_t temp[(MAX_STRING_LENGTH / 4)];                         //Declares temporary block in memory
        for (i = 0; i < (MAX_STRING_LENGTH / 4); i++)                   //Initializes temporary block to all 0
                temp[i] = 0;
        bool match = true;
        for (i = 0; i < (MAX_STRING_LENGTH / 4); i++)                   //Reads name section of block to temporary
            temp[i] = readEeprom(start + i);
        for (i = 0; i < MAX_STRING_LENGTH; i++) {                       //Compares the char to the searched name to prove that it does not match
            if ((temp[i / 4] << ((i % 4)) * 8) >> 3 * 8 != name[i])
                match = false;
            if (name[i] == '\0')
                break;
        }
        if (match)                                                      //Returns index if match is found (This only finds the first match)
            return index;
    }
    return 255;                                                         //Returns 255 if no match is found
}

//Return the addr and data for a command as a 16bit item | addr | data |
uint16_t getCommand(char * name)
{
    uint8_t index = getIndex(name);                                     //Looks for index of command with name
    if (index != 255) {                                                 //Checks that index was found
        uint8_t start = (index * ((MAX_STRING_LENGTH / 4) + 1)) + 1;    //Calculates the current starting location (size * blocksize) + 1
        uint32_t temp = readEeprom(start + (MAX_STRING_LENGTH / 4));    //Reads addr and data word from EEPROM
        return temp & 0xFFFF;                                           //Returns lower 16 bits of word | addr | data |
    } else
        return 0;                                                       //Returns 0 if index was not found
}

//Prints info for a command given the block index
void getName(uint8_t index)
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint16_t start = (index * ((MAX_STRING_LENGTH / 4) + 1)) + 1;       //Calculates the current starting location (size * blocksize) + 1
    if (index < size) {                                                 //Verifies that the index is less than the current number of saved commands
        uint8_t i;
        uint32_t temp[((MAX_STRING_LENGTH / 4) + 1)];                   //Declares temporary block in memory
        for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)             //Initializes temporary block to all 0
            temp[i] = 0;
        char name[MAX_STRING_LENGTH];
        for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)             //Reads block to temporary
            temp[i] = readEeprom(start + i);
        for (i = 0; i < MAX_STRING_LENGTH; i++)                         //Decodes name words to char array
            name[i] = (temp[i / 4] << ((i % 4)) * 8) >> 3 * 8;
        putsUart0(name);
        putcUart0('\n');
    }
}

//Prints info for a command given the block index
void infoCommand(uint8_t index)
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint16_t start = (index * ((MAX_STRING_LENGTH / 4) + 1)) + 1;       //Calculates the current starting location (size * blocksize) + 1
    if (index < size) {                                                 //Verifies that the index is less than the current number of saved commands
        uint8_t i;
        uint32_t temp[((MAX_STRING_LENGTH / 4) + 1)];                   //Declares temporary block in memory
        for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)             //Initializes temporary block to all 0
            temp[i] = 0;
        char name[MAX_STRING_LENGTH];
        for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)             //Reads block to temporary
            temp[i] = readEeprom(start + i);
        for (i = 0; i < MAX_STRING_LENGTH; i++)                         //Decodes name words to char array
            name[i] = (temp[i / 4] << ((i % 4)) * 8) >> 3 * 8;
        putsUart0(name);
        uint8_t add = (temp[(MAX_STRING_LENGTH / 4)] >> 8) & 0xFF;      //Decodes the address these are bits 15:8
        uint8_t data = temp[(MAX_STRING_LENGTH / 4)] & 0xFF;            //Decodes the data these are bits 7:0
        putsUart0("\nADDRESS: ");                                       //Prints the address in Binary
        uint8_t bit;
        for (bit = 8; bit > 0; bit--) {
            if ((1 << (bit - 1)) & add)
                putcUart0('1');
            else
                putcUart0('0');
        }
        putsUart0("\nDATA: ");                                          //Prints the data in Binary
        for (bit = 8; bit > 0; bit--) {
            if ((1 << (bit - 1)) & data)
                putcUart0('1');
            else
                putcUart0('0');
        }
    } else
        putsUart0("INVALID LOCATION");
    putcUart0('\n');
}

//Prints info for a command given the name
void infoCommandName(char * name)
{
    uint8_t i = getIndex(name);                                         //Gets the index of the block for this name
    if (i == 255)                                                       //Checks if index was found
        putsUart0("No Entry Found\n");
    else
        infoCommand(i);                                                 //Prints the info for that index
}

//Erases a block of memory leaving it empty given the index
void eraseCommand(uint8_t index)
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint16_t start = (index * ((MAX_STRING_LENGTH / 4) + 1)) + 1;       //Calculates the current starting location (size * blocksize) + 1
    uint8_t i;
    for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)                 //Writes over block with all 0
        writeEeprom(start + i, 0);
}

//Erases a block of memory leaving it empty given the name
void eraseCommandName(char * name)
{
    uint8_t i = getIndex(name);                                         //Gets index of block for name
    if (i == 255)                                                       //Checks if name was found
        putsUart0("No Entry Found\n");
    else
        eraseCommand(i);                                                //Erases block of memory at index
}

//Returns the index of a command given the addr and data
uint8_t findCommand(uint8_t addrOG, uint8_t dataOG)
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint8_t index;
    for (index = 1; index <= size; index++) {                           //Increments through all blocks of EEPROM
        uint16_t start = (index * ((MAX_STRING_LENGTH / 4) + 1));       //Calculates the current starting location (size * blocksize) + 1
        uint32_t temp = readEeprom(start);                              //Reads addr and data word | 00000000 | 00000000 | addr | data |
        uint8_t addr = (temp >> 8) & 0xFF;                              //Decodes the address these are bits 15:8
        uint8_t data = temp & 0xFF;                                     //Decodes the data these are bits 7:0
        if (addr == addrOG && data == dataOG)                           //Looks for a match and return index if found
            return index - 1;
    }
    return 255;                                                         //Returns 255 if not found
}

//Prints a list of all saved commands in EEPROM
void printCommands()
{
    uint32_t size = readEeprom(SIZE_LOC);                               //Grabs current number of saved commands
    uint8_t index;
    putsUart0("SAVED CODES:\n");
    for (index = 0; index < size; index++) {                           //Increments through all blocks of EEPROM
        uint16_t start = (index * ((MAX_STRING_LENGTH / 4) + 1)) + 1;       //Calculates the current starting location (size * blocksize) + 1
        uint8_t i;
        uint32_t temp[((MAX_STRING_LENGTH / 4) + 1)];                   //Declares temporary block in memory
        for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)             //Initializes temporary block to all 0
            temp[i] = 0;
        for (i = 0; i < ((MAX_STRING_LENGTH / 4) + 1); i++)             //Reads block to temporary
            temp[i] = readEeprom(start + i);
        char name[MAX_STRING_LENGTH];
        for (i = 0; i < MAX_STRING_LENGTH; i++)                         //Decodes name words to char array
            name[i] = (temp[i / 4] << ((i % 4)) * 8) >> 3 * 8;
        if (name[0] != 0) {                                             //Checks if this is an erases block location
            putsUart0(name);
            uint8_t add = (temp[(MAX_STRING_LENGTH / 4)] >> 8) & 0xFF;  //Decodes the address these are bits 15:8
            uint8_t data = temp[(MAX_STRING_LENGTH / 4)] & 0xFF;        //Decodes the data these are bits 7:0
            putsUart0("\tADDRESS: ");                                   //Prints the address in Binary
            uint8_t bit;
            for (bit = 8; bit > 0; bit--) {
                if ((1 << (bit - 1)) & add)
                    putcUart0('1');
                else
                    putcUart0('0');
            }
            putsUart0("\tDATA: ");                                      //Prints the data in Binary
            for (bit = 8; bit > 0; bit--) {
                if ((1 << (bit - 1)) & data)
                    putcUart0('1');
                else
                    putcUart0('0');
            }
            putcUart0('\n');
        }
    }
}
