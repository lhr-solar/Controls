*****************
Contactors Driver
*****************

The contactor driver is responsible for communication with the array and motor controller precharge bypass contactors. In essence, it controls whether the solar array and the motor controller are electrically connected to the rest of the car after their respective precharge processes.

The contactors can be set using a blocking or non-blocking interface. The blocking interface will wait until the contactor can be set, while the non-blocking interface will return early if another thread is currently setting the contactor.

.. autodoxygenfile:: Contactors.h
   :project: doxygen
