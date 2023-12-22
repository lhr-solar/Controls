*****************
Command Line Task
*****************

This file contains the command line interface, which is used to send commands to the car (and receive feedback) over UART. This is mainly useful for **debugging purposes only**, which is why it isn't enabled by default in :doc:`Main`. 

The command line interface is a simple task that reads characters from the serial port and parses them into commands. The commands are then executed by calling the appropriate function. The command line interface is also responsible for printing out the prompt and the results of commands.

The command line can be used to set/get the state of the car, including:

* Sending/Reading to the CANbus (see :doc:`../Drivers/CANBus`)
* Setting/Reading the state of the Contactors (see :doc:`../Drivers/Contactors`)
* Setting/Reading the value of any GPIO inputs/outputs (see :doc:`../Drivers/Minions`)
* Reading the value of the Pedals (see :doc:`../Drivers/Pedals`)

    LHRS Controls Command Line:
    For help, enter [help]
    
    Format is: cmd [param, ...]
    Commands and their params are as follows:
    
    CANbus_Send (non)blocking motor/car 'string' - Sends a CAN message with the string data as is on the determined line
	
    CANbus_Read (non)blocking motor/car - Reads a CAN message on the detemined line
		
    Contactors_Get array_c/array_p/motor_c - Gets the status of determined contactor
		
    Contactors_Set array_c/array_p/motor_c on/off (non)blocking - Sets the determined contactor
	
    Contactors_Enable  array_c/array_p/motor_c - Enables the determined contactor
		
    Contactors_Disable  array_c/array_p/motor_c - Disables the determined contactor
		
    Minions_Read 'input' - Reads the current status of the input
	
    Minions_Write `output` on/off - Sets the current state of the output
	
    Pedals_Read accel/brake - Reads the current status of the pedal