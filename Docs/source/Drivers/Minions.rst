.. _minions-driver:

.. _minion:

**************
Minions Driver
**************

The Minions driver is responsible for all lights and switches in the system, and in particular those that are present on the minion board. The driver implements the SPI protocol necessary to talk to the GPIO expander on the minion board. The driver exposes the following public interface:

The driver defines two enums: ``switches_t`` and ``light_t``. The former is used to distinguish switches in function calls, and the latter is the same for the lights. The exct enum values can be found in ``Drivers/Inc/Minions.h``.

The following public functions use these types heavily.

``void Minions_Init(void)`` — Creates a mutex to ensure that SPI access are thread-safe. Initializes SPI, as well as calling private functions to initialize both the lights and the switches seperately. Finally, the function sets up a filter to make switch readings more reliable over SPI. See :ref:`minions-impl` for more information about the filter.

``State Switches_Read(switches_t sw)`` — Reads the state of the given switch as stored in the driver. This does *not* query the hardware.

``State Lights_Read(light_t light)`` — Reads the state of the given light as stored in the driver. This does *not* query the hardware.

``void Switches_UpdateStates(void)`` — Queries GPIO pins and SPI to get the most up-to-date switch states. This function acquires the mutex to ensure that it is thread-safe. It also uses the filter (see :ref:`minions-impl`) to ensure that that switches connected over SPI return stable values. This function should be called fairly often (currently called every 10 ms) in order to make sure that we always have the most recent switch states stored by the driver.

``void Lights_Set(light_t light, State state)``

``uint16_t Lights_Bitmap_Read(void)`` — Returns a bitmap of light values, as stored by the driver. The least significant bit corresponds to a ``light_t`` value of zero.

``void Lights_Toggle(light_t light)`` — Toggles the given light.

``void Lights_Toggle_Set(light_t light, State state)`` — Set a specific light in the internal toggle bitmaps. This is used for lights that need to be toggled often, but can be easily extended in the future if necessary.

``State Lights_Toggle_Read(light_t light)`` — Reads the state of the internal toggle bitmap for the given light. This simplifies application-level toggling logic greatly.

``uint8_t Lights_Toggle_Bitmap_Read(void)`` — Returns the full internal toggle bitmap. This isn't used by any application code right now, but it might find some use in the future.


.. _minions-impl:

Implementation Details
======================

Some of the switches and lights are connected through GPIO. These are the ignition switch, the headlights, and the external blinkers. The rest of the lights and switches are on the minion board and connected through SPI. **NOTE: CURRENTLY, THE LIGHTS ON THE MINON BOARD ARE NON-FUNCTIONAL; THEY'RE SHOWN ON THE DISPLAY INSTEAD. THE SPI COMMANDS ARE STILL SENT, HOWEVER.**

The minion board is placed far away from the Controls leaderboard. As such, the SPI wires connecting the two are unusually long. This results in quite a bit of interference, which makes the lights on the minion board quite unreliable. The switches still work reliably due to the median filter used. The median filter keeps the last ten samples for each switch, and uses the median (since a switch is either on or off, this is equivalent to a majority check i.e. do we have more ones or more zeros) to determine the used state for each switch. This filter will hopefully be made redundant by a hardware redesign of the minon board to leaderboard connection.

.. doxygengroup:: Minions
   :project: doxygen
   :path: "/doxygen/xml/group__Minions.xml"
