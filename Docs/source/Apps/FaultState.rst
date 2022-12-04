****************
Fault State Task
****************

The fault state task was selected to be the highest priority task in the system in order to ensure that it can run immediately when a fault is encountered. The task waits on the fault state semaphore to be signaled, and calls ``EnterFaultState()`` when it does. The fault state task differentiates between recoverable and non-recoverable faults based on their origin and how often they've been raised since the system was started.

``FaultBitmap``
===============

The fault bitmap keeps track of the type of error that was encountered. The types of errors represented include OS errors, motor controller errors, and display errors. Check ``Tasks.h`` for the full definition of the bitmap. Based on the value of this bitmap, the fault state task can select what action to take based on the fault type. Furthermore, tritium errors are supplemented by a another bitmap that contains more specific information about the fault. The motor controller driver maintains this bitmap, and fault state uses it to discriminate between recoverable, non-recoverable, and ignorable errors. Check ``ReadTritium.h`` for the full definition of this bitmap.


Non-recoverable Faults
======================

Non-recoverable faults put the car in a state that cannot be safely rolled back to an operational state. This mainly includes OS faults, as well as motor controller overvoltage and overcurrent faults. It also includes recoverable faults that occur too often, see :ref:`recoverable`. When a non-recoverable fault is encountered, the controls fault light is activated (as are the blinkers and brake lights, for good measure) and the array and motor contactors are open. The system can only be reset by powering down and then powering up again.

.. _recoverable:

Recoverable Faults
==================

Recoverable faults leave the car in a consistent state from which the system can proceed without being reset. They are, however, still faults, and so the system doesn't ignore them completely. The two recoverable faults in the system are hall sensor errors (in the motor) and motor controller intialization errors. These are recoverable since the system can just try again. If these faults occur a few times, this may indicate a hardware failure somewhere (such as a loose connection or a faulty hall sensor) which is clearly non-recoverable. In order to handle this, recoverable faults have a trip counter, which is incremented every time the fault occurrs. If the trip counter exceeds some pre-determined value, a non-recoverable fault is manually triggered.