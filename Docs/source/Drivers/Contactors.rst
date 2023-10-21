*****************
Contactors Driver
*****************

The contactor driver is responsible for communication with the array and motor controller precharge bypass contactors. In essence, it controls whether the solar array and the motor controller are electrically connected to the rest of the car after their respective precharge processes. The public interface of the driver is as follows:

The contactors are refered to by the type ``contactor_t``, defined below::

    typedef enum {
        ARRAY_PRECHARGE = 0,
        MOTOR_CONTACTOR,
        NUM_CONTACTORS
    } contactor_t

Most of the functions below take a ``contactor_t`` parameter in order to determine which contactor to operate on.


``void Contactors_Init(void)`` — Initializes the GPIO pins that control the contactors. It also Initializes an internal data structure to keep track of whether the contactors are enabled or not: they all start off in the disabled state. The function also initializes a mutex that must be held before any contactor state changes are made. 

``bool Contactors_Get(contactor_t contactor)`` — Gets the state of a given contactor. This operation simply queries the GPIO pin state, and as such doesn't require acquiring the mutex lock.

``ErrorStatus Contactors_Set(contactor_t contactor, bool state, bool blocking)`` — Sets the state of a given contactor.  To keep this operation thread safe, the function attempts to acquire the mutex before proceeding, releasing it at the end. If blocking is true, the call will block until the mutex is acquired; else, the function will return an error without waiting.
This function is currently used to set the motor contactor on if the ignition switch is in position two (main.c), and sets the array contactor on if the ignition is in position one and BPS allows it (ReadCarCAN.c).