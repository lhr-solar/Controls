***************
Debug Dump Task
***************

This file contains the Debug Dump Task, which dumps the current state of the vehicle to UART. This is mainly useful for **debugging purposes only**, which is why it isn't enabled by default in :doc:`Main`. 

This can be used as a simpler version of the Command Line Interface to view periodic reads from each input to the MCU. This includes:

* Pedals (see :doc:`../Drivers/Pedals`)
* Switches, Buttons, and Lights (see :doc:`../Drivers/Minions`)
* Contactors (see :doc:`../Drivers/Contactors`)
* FSM Mode Information (see :doc:`SendTritium`)