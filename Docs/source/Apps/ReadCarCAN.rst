*****************
Read Car CAN Task
*****************

The Read Car CAN task reads the car's CANBus for values relevant to Controls. Currently we parse four types of messages: BPS trip, BPS contactor state, Supplemental Voltage, and State of Charge.

Ignition Sequence
=================

ReadCarCAN handles precharge for both the array and motor. The ignition switch has four positions: OFF, LV_ON (low voltage on), ARR_ON (array on), and MOTOR_ON. 

*   When turned to ARR_ON, BPS is meant to close their main HV Array Contactor, and ReadCarCAN will detect this change and start the precharge sequence for the Array. Array precharge is a delay of PRECHARGE_ARRAY_DELAY ms (see below).
*   When turned to MOTOR_ON, ReadCarCAN will make sure that both HV+ and HV- contactors are closed and start the precharge sequence for the Motor. Motor precharge is a delay of PRECHARGE_PLUS_MINUS_DELAY ms (see below).

Other messages
==============

We also expect to receive supplemental voltage and state of charge messages from BPS. Both of these are simply stored to show on the display.

Implementation Details
======================

The Read Car CAN task uses RTOS timers to make sure message timings remain appropriate. A watchdog is pet on BPS contactor state message receive to ensure that if communication is lost with BPS, the system disables the contactors.

In order to avoid charging in unsafe conditions, a saturation buffer is used to require that charge enable messages are sufficiently consistent.

.. doxygengroup:: ReadCarCAN
   :project: doxygen
   :path: "/doxygen/xml/group__ReadCarCAN.xml"
