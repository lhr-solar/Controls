***********
Extra Files
***********

Some files in the Apps folder don't contain any task code, but instead contain higher level interfaces, calibration data, and supporting structures. Some of these files will likely be relocated in the future.

.. _can-queue:

=========
CAN Queue
=========

Multiple tasks need to send messages over the car CAN bus. Sending is rather slow, so a queue is employed to absorbed the bursty nature of sending CAN messages. This way, multiple tasks can just post can messages to the queue, and another task (SendCarCAN) takes care to consume and send them to the CAN bus.

The public interface of the file includes:

``void CAN_Queue_Init(void)`` — Initialize the queue and its associated mutex.

``ErrorStatus CAN_Queue_Post(CANDATA_t message)`` — Used by tasks that want to post CAN messages. The messages are held in a FIFO queue.

``ErrorStatus CAN_Queue_Pend(CANDATA_t *message)`` — Used by SendCarCAN to consume the queue. This function blocks the caller until the queue contains at least one message, at which point it will return message and remove it from the queue.

================
Pedal to Percent
================

This file contains an array for converting a pedal percentage (the index into the array) into a float between 0 and 1. The mapping is currently linear, but this can be changed. The file was created in order to avoid costly floating point operations (since we've turned the floating point unit off), and should really be somewhere else (perhaps the driver layer).

=====
Tasks
=====

This file contains the storage for all general tasks related objects, including stacks, TCBs, mutexes, semaphores; as well as global variables such as ``RegenEnable`` and ``StateOfCharge``. All task files include ``Tasks.h`` and thus have access to them.