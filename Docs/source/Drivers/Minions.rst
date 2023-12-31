**************
Minions Driver
**************

The Minions driver is responsible for most of the lights and switches in the system, and in particular those that are present on the Controls Lighting board. The driver uses GPIO to set/read pins on the MCU, which in turn are connected to the Lighting Board.

Implementation Details
======================

All of the switches and lights that are controllable via software are connected through GPIO. These include:
*   Ignition Switch 1 & 2 (ARR_ON and MOTOR_ON)
*   Regen Enable Button
*   Gear Switch (Forward and Reverse) (note: if both Forward and Reverse are off, the gear switch is assumed to be in Neutral state)
*   Cruise Enable Button
*   Cruise Set Button
*   Brakelight

.. doxygenfile:: Minions.h
   
