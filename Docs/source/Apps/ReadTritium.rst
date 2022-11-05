*****************
Read Tritium Task
*****************

In its current iteration, the Read Tritium task forwards all incoming messages from motor CAN to car CAN. It does this using :ref:`can-queue`: The task posts messages to the queue, which are then read out by the SendCarCAN task.