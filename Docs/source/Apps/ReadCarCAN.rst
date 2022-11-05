*****************
Read Car CAN Task
*****************

The Read Car CAN task reads the car's CANBus for values relevant for Controls. Currently we parse three types of messages: charge enable, supplemental voltage, and state of charge. All three values are sent by BPS.

Charge Enable
=============

The BPS sends the Controls system a charge enable message that lets us know if its safe to charge the battery. This is important, since regen braking sends current from the motor to the battery. In order to ensure that we're never charging when it's not safe, a watchdog is employed: it checks that a message is received from BPS every few seconds, and disables charging if not. This is to account for any communication issues or anything else that might bar us from receving the message.

Other messages
==============

We also expect to receive supplemental voltage and state of charge messages from BPS. We simply store both to show on the display.

**This task will be greatly simplified when/if timers are successfully implemented.**