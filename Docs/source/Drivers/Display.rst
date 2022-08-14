**************
Display Driver
**************

**Note: This driver is about to go through a major overhaul. If the docs don't seem to match the code, let someone know!**

The display driver is responsible for all interactions with the display. As such, it includes many functions to set various screen elements' values. The public interface of the display driver is as follows:

``void Display_Init(void)`` — Makes sure UART is initialized, and then sets the display background color.

``ErrorStatus Display_SetMainView(void)`` — Sets the display to its main (non-precharged view). In general, we always want to be in this mode, so this function gets called very early on in the application code.

``ErrorStatus Display_CruiseEnable(State on)`` — Sets the cruise enable component to the given state

``ErrorStatus Display_SetVelocity(float vel)`` — Sets the velocity component to the given value. ``vel`` is given in meters per second, but display in miles per hour (with one digit after the decimal point)

``ErrorStatus Display_SetSBPV(uint16_t mv)`` — Sets the battery pack voltage component. Displayed to the driver with one digit after the decimal point.

``ErrorStatus Display_SetError(int idx, char *err)`` — Deprecated. Do not use.

``ErrorStatus Display_NoErrors(void)`` — Deprecated. Do not use.

``ErrorStatus Display_SetChargeState(uint32_t chargeState)`` — Sets the charge component to a given value. The format of this value is set by BPS, and displayed with one digit after the decimal point.

``ErrorStatus Display_SetRegenEnabled(State ChargeEnabled)`` — Sets the regen enable display component, letting the driver know whether regen is available to them at the moment.

``ErrorStatus Display_SetLight(uint8_t light, State on)``— Sets a light component to determined by the ``light`` to the given state. Whenever (and if) we get dedicated LEDs for blinkers, headlights, etc. this function will be deprecated.

``ErrorStatus Display_SetGear(State fwd, State rev)``— Sets the gear component based on the given ``fwd`` and ``rev`` states. If neither are one, a neutral gear is displayed. If both are on, an error is returned.


Internal Details
================

The display expects command strings over UART. The driver operates by constructing these strings in chunks and sending them over UART. For example say we have the component ``CHARGE_STATE``, and we wish to change its text to display 37.2. Internally, we'd first call ``updateIntValue(CHARGE_STATE, TXT, 372)``. This calls ``sendStartOfAssignment``, which fetches the string representation of the first two arguments (``x1`` and ``txt`` in this case), and then constructs the string ``"x1.txt="``, sending over UART. To complete the statement, ``updateIntValue`` then sends the number 372 and a message terminator. At this point the command is complete, and the display sends a return value, which the function interprets as success or failure.

The rest of the driver is built upon these basic building blocks. **WHENEVER THIS DRIVER IS UPDATE, THESE ARE EXPECTED TO CHANGE SIGNIFICANTLY.** This interface is not public for a reason, so don't attempt to use it from any application-level code.

Components are defined in an enum, the value of which is used to index an array of strings (**NOTE: THIS WILL BE CHANGED IN THE NEAR FUTURE**). The list of enum values is long and subject to change, so it won't be reproduced here. Check ``Drivers/Src/Display.c`` for details.