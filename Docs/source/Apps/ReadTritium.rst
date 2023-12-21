*****************
Read Tritium Task
*****************

In its current iteration, the Read Tritium task forwards all incoming messages from Motor CAN to CarCAN. It does this using the FIFO defined in :doc:`SendCarCAN`. The task posts messages to the queue, which are then read out by the SendCarCAN task.

.. autodoxygenfile:: ReadTritium.h
    :project: doxygen