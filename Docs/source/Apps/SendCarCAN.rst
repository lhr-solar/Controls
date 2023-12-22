*****************
Send Car CAN Task
*****************

The Send Car CAN task is a simple queue consumer task. Multiple tasks that need to write the the car CAN bus; in order to do this safely, they append their messages to a CAN queue. The Send Car CAN task simply pends on this queue and forwards messages to the Car CAN bus when any arrive.

The tasks that produce messages for the SendCarCAN queue include:
* :doc:`ReadTritium` (all messages on MotorCAN bus are echoed across CarCAN bus)
* :doc:`SendTritium` (the current FSM state is echoed across CarCAN bus for logging)

Put IO State Task
=================

The Put IO State task puts the current IO state on the CAN bus. It is used to send the IO state to Data Acquisition (for logging purposes) and the BPS (for ignition sequence purposes). Currently, it is written within `SendCarCAN.c`. It is a separate task from SendCarCAN (subject to change).

API
===
.. doxygenfile:: SendCarCAN.h
    :project: doxygen