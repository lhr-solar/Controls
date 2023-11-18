******************
Hardware Overview
******************

The Controls system is composed of:

*   A motor, driving the car
*   A motor controller, sending commands to the motor and keeping it in a safe operating range
*   Two seperate PCBs, for car control and switch management
*   Various switches and lights

=====
Motor
=====

A high-efficiency motor from Mitsuba. Three-phase, 96V nominal (160V max), 2000 watt nominal
(5000 watt max).

================
Motor Controller
================

Wavesculptor 22 from Prohelion. 14kw max output, 140V max input.

====
PCBs
====

------------
Leader Board
------------

The brains of Controls. Contains the microcontroller running our code (STM32f413),
CAN tranceivers, USB chip, analog filtering circuits, and voltage regulators.

------------
Minion Board
------------

Contains driving transistors and connectors for external lights, as well as inputs
and outputs for internal connections such as indicators.

===================
Lights and Switches
===================