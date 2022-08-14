*****************
Contactors Driver
*****************

The contactor driver is responsible for communication with the array and motor contactors. In essence, it controls whether the solar array and the motor are electrically connected to the rest of the car. The public interface of the driver is as follows:

The contactors are refered to by the type ``contactor_t``, defined below::

    typedef enum {
        ARRAY_CONTACTOR = 0,
        ARRAY_PRECHARGE,
        MOTOR_CONTACTOR,
        NUM_CONTACTORS
    } contactor_t

Most of the functions below take a ``contactor_t`` parameter in order to determine which contactor to operate on.


``void Contactors_Init(void)`` — Initializes the GPIO pins that control the contactors. It also Initializes an internal data structure to keep track of whether the contactors are enabled or not: they all start off in the disabled state. The function also initializes a mutex that must be held before any contactor state changes are made. 

``State Contactors_Get(contactor_t contactor)`` — Gets the state of a given contactor. This operation simply queries the GPIO pin state, and as such doesn't require acquiring the mutex lock.

``ErrorStatus Contactors_Set(contactor_t contactor, State state)`` — Sets the state of a given contactor. The change only occurs if ``contactor`` is enabled. To keep this operation thread safe, the function attempts to acquire the mutex before proceeding, releasing it at the end.

``void Contactors_Enable(contactor_t contactor)`` — Enables the given contactor, allowing it to be modified by ``Contactor_Set``

``void Contactors_Disable(contactor_t contactor)`` —  Disables the given contactor, barring ``Contactors_Set`` from modifying it.