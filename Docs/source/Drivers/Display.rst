**************
Display Driver
**************

The display driver is responsible for all interactions with the display.
It includes functions to reset the display and to send commands to it.
It also defines a display command struct, from which a command string is
assembled and then sent out to the display.

Command Structure
-----------------

A more detailed description of the command structure is given below. For more information on the command syntax, see the `Nextion Documentation <https://nextion.tech/instruction-set/>`_:

* ``char* compOrCmd`` — Either a component or a command, depending on the next few values

* ``char *attr`` — When the above is a component, the attribute of that component. Else, ``NULL``

* ``char *op`` — The operation when the above two are set, usually ``=``. Else, ``NULL``

* ``numArgs`` — The number of arguments the command takes. Should be 1 if setting component

* ``argTypes[]`` — A list of the argument types to follow

* ``args`` — The actual arguments for the command (strings or ints)

.. doxygengroup:: Display
   :project: doxygen
   :path: "/doxygen/xml/group__Display.xml"
