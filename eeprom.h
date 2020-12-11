// EEPROM functions
// Nathan Fusselman

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    -

#ifndef EEPROM_H_
#define EEPROM_H_

#define MAX_STRING_LENGTH 16                //Max size of name string, optimal if a multiple of 4
#define SIZE_LOC 0                          //Location in EEPROM of the Size Variable

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initEeprom(void);                      //Provided function by my main man Losh that initializes the EEPROM Module.
void writeEeprom(uint16_t, uint32_t);       //Provided function by my main man Losh that writes a 32bit word to the EEPROM at a 16bit address location.
uint32_t readEeprom(uint16_t);              //Provided function by my main man Losh that reads a 32bit word from the EEPROM at a 16bit address location.
void clearAllCommands(void);                //Sets the Size of saved commands to 0 so that it will start rewriting at the first block in the EEPROM.
void addCommand(char *, uint8_t, uint8_t);  //Creates and adds block to EEPROM containing the command data.
uint8_t getIndex(char *);                   //Returns the entry index of a block saved in the EEPROM
uint16_t getCommand(char *);                //Return the addr and data for a command as a 16bit item | addr | data |
void getName(uint8_t);
void infoCommand(uint8_t);                  //Prints info for a command given the block index
void infoCommandName(char *);               //Prints info for a command given the name
void eraseCommand(uint8_t);                 //Erases a block of memory leaving it empty given the index
void eraseCommandName(char *);              //Erases a block of memory leaving it empty given the name
uint8_t findCommand(uint8_t, uint8_t);      //Returns the index of a command given the addr and data
void printCommands(void);                   //Prints a list of all saved commands in EEPROM

#endif
