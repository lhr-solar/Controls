*******************
Update Display Task
*******************

The update display task maintains a queue of command structures (see :ref:`cmd`), allowing other tasks to submit commands and sending out commands to the display as time allows. See ``UpdateDisplay.h`` for details of the public interface (there are various functions to set different components of the display). Since these are just wrapper functions, the implementation details will be explained next.

Internal implementation
-----------------------

In a similar way to :ref:`can-queue`, the task implements a queue of command structures. Tasks that wish to write the display call one of the wrapper functions exposed in the public interface. Internally, all of these functions construct a command structure and then call ``UpdateDisplay_PutNext(Display_Cmd_t cmd)``, which places the command structure in the queue and notifies UpdateDisplay. Another internal function, ``UpdateDisplay_Error_t UpdateDisplay_PopNext(void)``, gets called by UpdateDisplay. It blocks on a semaphore until the queue is not empty, and then grabs the next command structure. It then parses the command and sends it out over UART. Check these functions in ``UpdateDisplay.c`` for more information.