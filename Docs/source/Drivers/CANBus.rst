.. _canbus:

*************
CANBus Driver
*************

The CANbus driver is responsible for all incoming and outgoing CAN communication on both CAN lines. The driver knows of more than a dozen different CAN messages it can send or receive, and can automatically determine their size and data type. The interface presented by the driver is as follows:

* ``ErrorStatus CANbus_Init(CAN_t bus)`` — Initialize the canbus given by the ``bus`` argument. The options are ``MOTORCAN`` and ``CARCAN``.

* ``ErrorStatus CANbus_Send(CANDATA_t CanData, CAN_blocking_t blocking, CAN_t bus)`` — Send The given ``CANDATA`` structure to ``bus``. If ``blocking`` is true, the call blocks until the message can be deposited in a vacant CAN mailbox. Else, the function will return an error if no mailbox is vacant.

* ``ErrorStatus CANbus_Read(CANDATA_t* data, CAN_blocking_t blocking, CAN_t bus)`` — Read a CAN message from ``bus`` into the given data structure. If ``blocking`` is true, wait until a CAN message is available to be read. Else, the function will return an error if no message is ready to be read.

Data Types
==========

CAN messages are sent from and received into ``CANDATA_t``, which contains the CAN ID, idx byte, and up to 8 data bytes. Internally, the driver also uses the ``CANLUT_T`` type, which is the entry type of a lookup table used to determine the data types used by incoming messages. See ``CANbus.h`` and ``CANLUT.c`` for details.

Implementation Details
======================

The microcontroller's CAN hardware block includes three sending and three receiving mailboxes, which act as a small queue of CAN messages. The driver will read and write to those mailboxes as long as they aren't empty or full, respectively, blocking otherwise. This is done since tasks that read and write CAN messages don't usually have anything else to do while waiting, which makes blocking fairly efficient. If a more important task such as :ref:`velocity` needed to send CAN messages, then perhaps it a software queue would've been used to supplement the hardware mailboxes.