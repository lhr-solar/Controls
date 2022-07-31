*************
Pedals Driver
*************

The Pedals driver is a thin wrapper over the ADC interface exposed by the BSP. It defines an enum that allows the driver to distinguish between the the accelerator pedal and the brake pedal. It also defines ``Pedals_Init()`` (which initializes the ADC) and ``Pedals_Read(pedal_t pedal)``, which takes a pedal and returns how far down it's pushed between 0 and 100.

The Pedals driver provides calibrates the ADC output in order to get sensible values for the pedals. These calibration parameters are provided as macro definitions in the header file, and can be easily changed if the pedal wiring changes or if further tuning is required.

**Note:** The brake pedal that is currently in the car isn't quite working, so it's been switched over to a GPIO pin. The Pedals driver is still used to access the brake pedal state, but it's now negative logic; a low pedal percentage means that the brake pedal is pressed down.