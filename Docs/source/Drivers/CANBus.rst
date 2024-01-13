.. _canbus:

*************
CANBus Driver
*************

The CANbus driver is responsible for all incoming and outgoing CAN communication on both CAN lines. The driver knows of more than a dozen different CAN messages it can send or receive, and can automatically determine their size and data type. The interface presented by the driver is as follows:

* ``ErrorStatus CANbus_Init(CAN_t bus)`` — Initialize the canbus given by the ``bus`` argument. The options are ``MOTORCAN`` and ``CARCAN``.

* ``ErrorStatus CANbus_Send(CANDATA_t CanData, bool blocking, CAN_t bus)`` — Send The given ``CANDATA`` structure to ``bus``. If ``blocking`` is true, the call blocks until the message can be deposited in a vacant CAN mailbox. Else, the function will return an error if no mailbox is vacant.

* ``ErrorStatus CANbus_Read(CANDATA_t* data, bool blocking, CAN_t bus)`` — Read a CAN message from ``bus`` into the given data structure. If ``blocking`` is true, wait until a CAN message is available to be read. Else, the function will return an error if no message is ready to be read.

Data Types
==========

CAN messages are sent from and received into ``CANDATA_t``, which contains the CAN ID, idx byte, and up to 8 data bytes. 
Internally, the driver also uses the ``CANLUT_T`` type, which is the entry type of a lookup table used to determine the data types used by incoming messages. 
This lookup table can also be used to determine the length of incoming messages if it is needed.
See ``CanBus.h`` and ``CANLUT.c`` for details.

Implementation Details
======================

The microcontroller's CAN hardware block includes three sending and three receiving mailboxes, which act as a small queue of CAN messages. 
The driver will write directly to the write mailboxes as long as they aren't full, and will block otherwise. 

The receive mailboxes are constantly emptied (by the CAN receive interrupt) 
into a software queue in the BSP layer in order to deepen the queue (we receive a lot of messages).
When reading, the driver will read directly from a software queue in the BSP layer. A non-blocking read will return an error if the queue is empty. A blocking read will block on a driver-layer semaphore, to be woken up when data is avilable.
Everytime the BSP software queue is posted to, a receive interrupt signals to a driver-layer semaphore that a message has been received. This allows any waiting tasks to wake up and read the message. 
This is done since tasks that read and write CAN messages don't usually have anything else to do while waiting, which makes blocking fairly efficient. 

.. doxygengroup:: CANbus
   :project: doxygen
   :path: "/doxygen/xml/group__CANBus.xml"
