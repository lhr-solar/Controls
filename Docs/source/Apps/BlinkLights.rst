*****************
Blink Lights Task
*****************

This tasks ensures that the blinkers blink at a reasonable frequency. The task simply reads the current state of each light and modifies it based on the state of that light's toggle property (see :ref:`minions-driver`). This task will likely be deprecated whenever lights that blink in hardware are used in the system.