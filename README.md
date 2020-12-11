# IR-Remote-Control-Interface
 Bidirectional NEC Remote Control Interface built for TM4C123GH6PM

 The University of Texas at Arlington | Junior Year | CSE 3442 | Embedded I
 This Project was programed in C for bare metal operation on the TI Tiva Development Board.
 This project was developed and created in CCStudio.

# Overview
 This project was designed to interface an NEC remote control with your computer
 by accepting input signals and transmitting the decoded information via UART to
 the connected computer. This device is also capable of transmitting saved codes
 to other nearby devices. The device is able to save 255 total codes in its
 internal EEPROM for non-volatile storage. This project could be expanded by
 allowing this device to communicate with a similar board that is connected to
 the internet and allow for remote non line of sight communication with IR
 capable devices.

# UART Commands
## HELP
    The help command displays all commands available to the end user in a list
    that shows what arguments are required and or optional.
    List of available commands:
      HELP
      LIST
      DECODE [ON/OFF]
      CLEAR
      LEARN <NAME> [ADDR] [DATA]
      INFO <NAME>
      ERASE <NAME>
      PLAY <NAME> ...
      SEND <ADDR> <DATA>
      SENDB <BUTTON>
      SENDE
      ALERT <GOOD/BAD/ERROR> <ON/OFF>
## LIST
    The list command will list out all saved commands that are stored in the
    EEPROM so that you know what can be discovered and be able to properly remove
    them if necessary.
## DECODE [ON/OFF]
    This command allows for you to enable or disable to printing or addresses
    and data bits when a button is pressed. This will default to off but can be
    toggled by just entering DECODE and can be forcefully set with DECODE ON or
    DECODE OFF.
## CLEAR
    The clear command is most helpful if you would like to all saved commands in
    the EEPROM. This is so that you can just start fresh.
## LEARN <NAME> [ADDR] [DATA]
    To be able to learn a new command and save it to the EEPROM you have a few
    options. Most simply if you want to learn from another remote you can simply
    run the command LEARN <NAME> and then press the button on the remote so that
    it can capture the input from the remote itself. Alternatively you can enter
    the command LEARN <NAME> [ADDR] [DATA] to directly specify the address and
    data values.
## INFO <NAME/ID>
    At some time you may want to see what is stored for the data at a location
    in the EEPROM or find an entry by its name. To do this you can use this
    command and enter either the ID or the name you have and the system will be
    able to print the address and data values in binary for that entry.
## ERASE <NAME/ID>
    You may need to also remove an item from the EEPROM without effecting other
    and to do this would would use the command ERASE the the ID or name following
    that. This will replace the location in memory with all 0’s making that name
    not able to be found as the string is represented by a NULL.
## PLAY <NAME>
    To play a command for another receiver to read or to send back to itself you
    will want to use the play command and specify the name of the item you would
    like to send. You can also place multiple names and the device will play
    each one consecutively.
## SEND <ADDR> <DATA>
    This is a debug command that allows for you to send an NEC packet with any
    address and data value you would like by sending this command.
## SENDB < BUTTON>
    This is an additional debug command that allows for you to send the code for
    any of the button on the 21 button remote by entering in the number for that
    button.
## SENDE
    This debug command allows for you to send a signal to another device that
    intentionally has an error with the inverted bits being backwards. This is
    so that you can ensure that your error checking works.
## ALERT <GOOD/BAD/ERROR> <ON/OFF>
    The ALERT command allows for you to enable and disable any of the 3 alerts
    on the system by specifying what one you want to change and telling it what
    state you want it to become.
