***********************
Motor Controller Driver
***********************

The motor controller driver is responsible for communicating with the motor over a dedicated CAN line. The driver exposes ``MotorController_Init(float busCurrentFractionalSetPoint)`` to initialize the motor CAN line and initialize the motor controller to the given set point.

Public Interface:

*   ``MotorController_Init(float busCurrentFractionalSetPoint)`` — Initializes the motor CAN line as well as the motor controller itself
*   ``MotorController_Drive(float newVelocity, float motorCurrent)`` — Queues the drive command to be sent to the motor controller
*   ``MotorController_Read(CANbuff *message)`` — Read the motor CAN line to get any updates from the motor. These include velocity and any sort of error codes.
*   ``MotorController_ReadVelocity(void)`` — The driver caches the last velocity value it read from the motor controller