****
UART
****

This module provides a low-level interface to two UART ports, intended for use of the display and USB communication. The implementation uses receive and transmit FIFOs, as well as receive and transmit callbacks in order to be able to send and receive longer messages without losing any data.

.. doxygenfile:: BSP_UART.h
   

