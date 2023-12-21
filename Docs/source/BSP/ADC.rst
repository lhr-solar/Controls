***
ADC
***

This module provides a low-level interface to two ADC channels, which are intended to be used for the accelerator and brake pedals. Internally, we use DMA to continuously copy the ADC values into an array that is served out.

.. autodoxygenfile:: BSP_ADC.h
   :project: doxygen
