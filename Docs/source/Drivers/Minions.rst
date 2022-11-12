.. _minions-driver:

**************
Minions Driver
**************

The Minions driver is responsible for some switches and one light. Specifically, the switches for ignition (1 and 2), regen, forward, reverse, cruise enable, and cruise set and the brakelight. The driver exposes the following public interface:

All the pins that are controlled by Minions are kept in an enum called ``MinionPin_t``. Each pin has a pinmask, port, and direction kept in a lookup table located in ``Drivers/Src/Minions.c`` which is used by Minions to interact with the ``BSP_GPIO`` functions.

``Minion_Error_t`` is an error enum that is passed into user functions and contains an error if something went wrong in the funciton. 

``MINION_ERR_YOU_READ_OUTPUT_PIN`` occurs when you read an output pin. The function will continue to do nothing.

``MINION_ERR_YOU_WROTE_TO_INPUT_PIN`` occurs when you try to change the output of an input pin. The function will continue to do nothing.

The following are the public functions:

``void Minion_Init(void);`` - Initializes input switches, output pins, and output mutex.

``bool Minion_Read_Input(MinionPin_t pin, Minion_Error_t* err);`` - Reads current state of specified input pin. Note: If you pass in a pin that is an output, then the function will do nothing and update the minion error enum.

``bool Minion_Write_Output(MinionPin_t pin, bool status, Minion_Error_t* mErr);`` - Writes given status to a specified output pin. Locks writing to all output pins with an output mutex. Note: If you pass in an input pin, then the funciton will do nothing and update the minion error enum.

.. _minions-impl:

Implementation Details
======================

There are other switches and lights in our system. However, they are not controlled by software and do not interact with Minions. Instead, they are controlled purely through hardware.