*****************
Send Car CAN Task
*****************

The send car CAN task is a simple queue consumer task. Multiple tasks that need to write the the car CAN bus; in order to do this safely, they append their messages to a CAN queue (see :ref:`can-queue`). The send car CAN tasks simply pends on this queue and forwards messages to the car CAN bus when any arrive.

