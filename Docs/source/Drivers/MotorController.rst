***********************
Motor Controller Driver
***********************

The motor controller driver is responsible for communicating with the motor over a dedicated CAN line. The driver exposes the following functions as part of its public interface:

``void MotorController_Init(float busCurrentFractionalSetPoint)`` — Initializes the motor CAN line as well as the motor controller itself. The sole argument is a floating point value between between ``0.0f`` and ``1.0f``, and is used to impose a current limit on top of the one preconfigured in the motor controller. If, for example, the motor controller is configured to draw a maximum of 50 amps, setting the argument to ``0.2f`` would now limit the motor controller to only drawing 10 amps. This was mainly useful for testing, but in general the value of ``1.0f`` should be passed.

``void MotorController_Drive(float newVelocity, float motorCurrent)`` — Queues the drive command to be sent to the motor controller. ``newVelocity`` should be given in meters per second, while ``motorCurrent`` should be given in amps. The drive command must be sent to the motor controller at least once every 250 milliseconds, otherwise it will shut down. To ensure this doesn't happen, it should be called roughly 100 ms while the car is in drive or reverse.

``ErrorStatus MotorController_Read(CANbuff *message)`` — Read the motor CAN line to get any updates from the motor. These include velocity, RPM, and any sort of error codes. The velocity and RPM values are internally stored by the motor controller. The error codes are currently forwarded to the car CAN line for logging by the data acquisition system, but this will likely change in the future, since it unnecessarily crowds the CAN bus.

``float MotorController_ReadVelocity(void)`` — The motor controller driver returns the latest cached velocity value obtained by a ``MotorController_Read`` call. As such, the obtained value is only as new as the last time the read call obtained a velocity value.

``float MotorController_ReadRPM(void)`` — The motor controller driver returns the latest cached RPM value obtained by a ``MotorController_Read`` call. As such, the obtained value is only as new as the last time the read call obtained an RPM value.
