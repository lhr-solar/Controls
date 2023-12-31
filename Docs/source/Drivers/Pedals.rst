*************
Pedals Driver
*************

The Pedals driver is a thin wrapper over the ADC interface exposed by the BSP.

The Pedals driver provides calibrates the ADC output in order to get sensible values for the pedals. These calibration parameters are provided in the c file, and can be easily changed if the pedal wiring changes or if further tuning is required.

**Note:** The brake pedal that is currently in the car isn't quite working, so it's been switched over to a GPIO pin. The Pedals driver is still used to access the brake pedal state, but it's now negative logic; a low pedal percentage means that the brake pedal is pressed down.

.. doxygenfile:: Pedals.h
   
