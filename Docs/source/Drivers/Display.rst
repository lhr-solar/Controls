**************
Display Driver
**************

The display driver is responsible for all interactions with the display. As such, it includes many functions to set various screen elements' values. The driver defines a command struct, which represents a command to be sent to the display. The driver exposes the following functions:

* ``Display_Error_t Display_Init(void)`` — Initializes UART and resets the display.

* ``Display_Error_t Display_Reset(void)`` — Sends the reset command to the display.

* ``Display_Error_t Display_Send(Display_Cmd_t cmd)`` — Send the given command to the display. In general, this function shouldn't be called directly. See FILLER for more details on where it's called.

* ``Display_Error_t Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode)`` — Display the fault page on the display, presenting ``osErrCode`` and ``faultCode`` to the user.

.. _cmd:

Command Structure
-----------------

A more detailed description of the command structure is given below. For more information on the command syntax, see the `Nextion Documentation <https://nextion.tech/instruction-set/>`_:

* ``char* compOrCmd`` — Either a component or a command, depending on the next few values

* ``char *attr`` — When the above is a component, the attribute of that component. Else, ``NULL``

* ``char *op`` — The operation when the above two are set, usually ``=``. Else, NULL

* ``numArgs`` — The number of arguments the command takes. Should be 1 if setting component

* ``argTypes[]`` — A list of the argument types to follow

* ``args`` — The actual arguments for the command (strings or ints)

.. doxygengroup:: Display
   :project: doxygen
   :path: "/doxygen/xml/group__Display.xml"
