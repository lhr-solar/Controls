***
SPI
***

This module provides low-level access to SPI, intended for communication with the minions board (**NOTE: a new minions driver is currently being written, so this use case might change**). Internally, we provide a spin-locking mechanism for short SPI messages (shorter than 8 bytes) and an OS level pend on longer ones.