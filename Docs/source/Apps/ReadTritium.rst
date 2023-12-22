*****************
Read Tritium Task
*****************

In its current iteration, the Read Tritium task forwards all incoming messages from Motor CAN to CarCAN. It does this using the FIFO defined in :doc:`SendCarCAN`. The task posts messages to the queue, which are then read out by the SendCarCAN task.

ReadTritium also facilitates reading velocity and error information in from the motor controller. Velocity is put on the display, and error information is used to determine if the motor controller is in a fault state. If the motor controller is in a fault state, the task will attempt to reset the motor controller. If the motor controller cannot be reset, the task will set the car to a fault state.

API
===
.. autodoxygenfile:: ReadTritium.h
    :project: doxygen