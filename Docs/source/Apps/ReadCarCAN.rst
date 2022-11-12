*****************
Read Car CAN Task
*****************

The Read Car CAN task reads the car's CANBus for values relevant to Controls. Currently we parse three types of messages: charge enable, supplemental voltage, and state of charge. All three values are sent by BPS.

Charge Enable
=============

The BPS sends the Controls system a charge enable message that lets us know if it's safe to charge the battery. This is important because regen braking sends current from the motor to the battery. In order to ensure that we're only charging when it's safe, a watchdog is employed: it checks that a message is received from BPS every few seconds and disables charging if one isn't. This is to account for communication issues or anything else that might bar us from receiving the message.

Other messages
==============

We also expect to receive supplemental voltage and state of charge messages from BPS. Both of these are simply stored to show on the display.

Implementation Details
======================

The Read Car CAN task uses timer callbacks to make sure message timings remain appropriate. More specifically, there is a timer that waits for the precharge contactor to be opened and closed by BPS before restarting the array, and another timer that ensures a charge enable message is received from BPS at least every half second. If no such message is received, the timer signals the fault state task to unblock and enter fault state, classifying it as one of the :ref:`recoverable`, and opening the contactors controlled by the system.

In order to avoid charging in unsafe conditions, a saturation buffer is used to require that charge enable messages are sufficiently consistent. See `ReadCarCAN.c` for more details, as this is subject to change.